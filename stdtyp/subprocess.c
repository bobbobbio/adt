// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <stdtyp/subprocess.h>
#include <stdtyp/file.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

create_error_body(subprocess_error);

void
subprocess_exec(const struct string_vec *arg_vec)
{
   create_cstr_array(argv, arg_vec);
   execvp(argv[0], (char * const *)argv);

   // failure
   ecrash_errno_error();
}

pid_t
subprocess_fork(void)
{
   pid_t pid = fork();
   if (pid == -1)
      panic("Failed to fork a child process");

   return pid;
}

struct error
subprocess_run(const struct string_vec *arg_vec, struct string *output)
{
   // Creates 2 file descriptors that are linked together writes to one go to
   // the other:
   // write_pipe ---> read_pipe
   int fds[2];
   if (pipe(fds) != 0)
      eraise(subprocess_error, "Failed to make pipe");
   create_file_fd(write_pipe, fds[1]);
   create_file_fd(read_pipe, fds[0]);

   pid_t pid = fork();
   if (pid == 0) { // child
      // set up the stdout file descriptor to point to our pipe's write end
      if (dup2(file_fd(&write_pipe), STDOUT_FILENO) == -1) {
         panic("Failed to link stdout to pipe");
      }
      if (dup2(file_fd(&write_pipe), STDERR_FILENO) == -1) {
         panic("Failed to link stderr to pipe");
      }
      // these are now not needed on the child, and the cleanup won't work
      ecrash(file_close(&write_pipe));
      ecrash(file_close(&read_pipe));
      subprocess_exec(arg_vec);
   } else { // parent
      // the write pipe has to close before reading
      ereraise(file_close(&write_pipe));
      int status;
      if (waitpid(pid, &status, 0) == -1) {
         eraise(subprocess_error, "Failed to wait for child");
      }

      if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS)
         eraise(subprocess_error,
            "Subprocess had non-zero exit status");

      ereraise(file_read(&read_pipe, output));
   }

   return no_error;
}
