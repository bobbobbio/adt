// Copyright 2015 Remi Bernotavicius. All right reserved.

// ptrace on OS X is a complete joke
#ifndef __APPLE__
#include <extyp/ptrace.h>
#include <error.h>
#include <stdtyp/file.h>
#include <stdtyp/subprocess.h>

#include <errno.h>
#include <stddef.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>

#define reg_offset(name) (void *)(offsetof(struct user_regs_struct, name))

adt_func_pod_body(ptracer_child);
map_gen_podk_body(pid_map, pid_t, ptracer_child);
adt_func_body(ptracer);

void
ptracer_init(struct ptracer *self)
{
   pid_map_init(&self->children);
}

void
ptracer_destroy(struct ptracer *self)
{
   pid_map_destroy(&self->children);
}

create_error_body(ptracer_exited_error);

void
ptracer_add_child(struct ptracer *self, pid_t pid)
{
   create(ptracer_child, child);
   child.pid = pid;
   pid_map_insert(&self->children, pid, &child);
}

a_warn_unused_result
static struct error
ptracer_wait_for_syscall(struct ptracer *self, struct ptracer_child **child_out)
{
   while (true) {
      iter_value (pid_map, &self->children, child)
         ptrace(PTRACE_SYSCALL, child->pid, 0, 0);

      int status;
      int pid = waitpid(-1, &status, 0);
      if (pid < 0)
         eraise_errno_error();

      // If for some reason this is for a child we don't know about, ignore it.
      if (!pid_map_contains(&self->children, pid))
        continue;

      int pstatus = status >> 16;

      if (pstatus == PTRACE_EVENT_FORK || pstatus == PTRACE_EVENT_VFORK ||
         pstatus == PTRACE_EVENT_CLONE) {
         // When a child forks, we have to add the new pid to the list of pids
         // we are tracking so we have associated state for it.
         int newpid;
         ptrace(PTRACE_GETEVENTMSG, pid, NULL, (long)&newpid);
         if (errno != 0)
            eraise_errno_error();
         ptracer_add_child(self, newpid);
      }

      if (WIFSTOPPED(status) && WSTOPSIG(status) & 0x80) {
         // This means a child is entering or exiting a syscall
         *child_out = pid_map_at(&self->children, pid);
         return no_error;
      }

      if (WIFEXITED(status)) {
         // This means a child actually exited, so we stop tracking it.
         pid_map_remove(&self->children, pid);
         if (pid_map_size(&self->children) == 0)
            eraise(ptracer_exited_error, "");
      }
   }
}

a_warn_unused_result
static struct error
ptracer_child_peek_user(struct ptracer_child *self, void *addr, long *data_out)
{
   *data_out = ptrace(PTRACE_PEEKUSER, self->pid, addr);
   if (errno != 0)
      eraise_errno_error();
   return no_error;
}

a_warn_unused_result
static struct error
ptracer_child_peek_data(struct ptracer_child *self, void *addr, long *data_out)
{
   *data_out = ptrace(PTRACE_PEEKDATA, self->pid, addr);
   if (errno != 0)
      eraise_errno_error();
   return no_error;
}

a_warn_unused_result
static struct error
ptracer_child_peek_data_string(
   struct ptracer_child *self,
   void *addr,
   struct string *res_out)
{
   string_clear(res_out);

   long paddr;
   ereraise(ptracer_child_peek_user(self, addr, &paddr));
   while (true) {
      long val;
      ereraise(ptracer_child_peek_data(self, (void *)paddr, &val));
      paddr += sizeof(long);

      for (char *c = (char *)&val; c - (char *)&val < sizeof(val); c++) {
         string_append_char(res_out, *c);
         if (*c == '\0')
            return no_error;
      }
   }

   panic("not reached");
}

a_warn_unused_result
static struct error
ptracer_child_got_syscall(struct ptracer_child *self)
{
   if (!self->in_syscall) {
      long syscall = -1;
      ereraise(ptracer_child_peek_user(self, reg_offset(orig_rax), &syscall));

      self->in_syscall = true;
      self->syscall = syscall;
   } else {
      self->in_syscall = false;

      // look for file access calls
      if (!(self->syscall == SYS_open || self->syscall == SYS_access ||
         self->syscall == SYS_stat || self->syscall == SYS_lstat)) {
         return no_error;
      }

      // get the return value
      long retval = -1;
      ereraise(ptracer_child_peek_user(self, reg_offset(rax), &retval));

      // get the argument
      create(string, path);
      ereraise(ptracer_child_peek_data_string(self, reg_offset(rdi), &path));

      // open returns a fd on success, and a negative value on failure
      if (retval < 0 && !string_starts_with(&path, strw("/")))
         afprintf(stderr, "TRACE: %s\n", print(string, &path));
   }

   return no_error;
}

void
ptracer_trace(struct ptracer *self)
{
   adt_assert(pid_map_size(&self->children) > 0);

   bool exited = false;
   while(!exited) {
      struct ptracer_child *child;
      ehandle(error, ptracer_wait_for_syscall(self, &child)) {
         if (error_is_type(error, ptracer_exited_error))
            return;
         else
            ecrash(error);
      }

      ecrash(ptracer_child_got_syscall(child));
   }
}

struct error
ptracer_subprocess(struct ptracer *self, const struct string_vec *cmd)
{
   // Must not be tracing anything yet
   adt_assert(pid_map_size(&self->children) == 0);

   pid_t child = subprocess_fork();
   if (child == 0) { // child
      // Tell parent we are to be traced
      ptrace(PTRACE_TRACEME);

      // Use this signal to tell our parent we are ready
      kill(getpid(), SIGSTOP);
      subprocess_exec(cmd);
   }

   // parent

   // Wait for child to be ready
   int status;
   waitpid(child, &status, 0);
   adt_assert(WIFSTOPPED(status));

   /*
    * PTRACE_O_TRACESYSGOOD (since Linux 2.4.6)
    *    When delivering system call traps, set bit 7 in the signal number
    *    (i.e., deliver SIGTRAP|0x80).  This makes it easy for the tracer to
    *    distinguish normal traps from those caused by a  system call.
    *
    *    If  the  PTRACE_O_TRACEFORK,  PTRACE_O_TRACEVFORK,  or
    *    PTRACE_O_TRACECLONE options are in effect, then children created by,
    *    respectively, vfork(2) or clone(2) with the CLONE_VFORK flag, fork(2)
    *    or clone(2) with the exit signal set to  SIGCHLD,  and other  kinds  of
    *    clone(2), are automatically attached to the same tracer which traced
    *    their parent.  SIGSTOP is delivered to the children, causing them to
    *    enter signal-delivery-stop after they exit the system call which
    *    created them.
    */
   ptrace(PTRACE_SETOPTIONS, child, 0,
      PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEFORK  | PTRACE_O_TRACEVFORK |
      PTRACE_O_TRACECLONE);

   if (errno != 0)
      eraise_errno_error();

   ptracer_add_child(self, child);

   return no_error;
}

#endif // __APPLE__
