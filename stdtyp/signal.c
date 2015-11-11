// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <string.h>

#include <stdtyp/signal.h>
#include <stdtyp/file.h>

struct signal_handler {
   void (*func)();
};
adt_func_pod_static(signal_handler);
vector_gen_static(signal_handler_vec, signal_handler);

static void
signal_handler_print(const struct signal_handler *self, struct string *dest)
{
   // do nothing...
}

struct signal {
   int num;
};
adt_func_pod_static(signal);

// strsignal can do this for you, but its not always available
// XXX This is only correct for x86, arm and most architectures
// (see man 7 signal)
char *signal_to_str[] = {
   /*  0 */   NULL,
   /*  1 */   "SIGHUP",
   /*  2 */   "SIGINT",
   /*  3 */   "SIGQUIT",
   /*  4 */   "SIGILL",
   /*  5 */   "SIGTRAP",
   /*  6 */   "SIGABRT",
   /*  7 */   "SIGBUS",
   /*  8 */   "SIGFPE",
   /*  9 */   "SIGKILL",
   /* 10 */   "SIGUSR1",
   /* 11 */   "SIGSEGV",
   /* 12 */   "SIGUSR2",
   /* 13 */   "SIGPIPE",
   /* 14 */   "SIGALRM",
   /* 15 */   "SIGTERM",
   /* 16 */   "SIGSTKFLT",
   /* 17 */   "SIGCHLD",
   /* 18 */   "SIGCONT",
   /* 19 */   "SIGSTOP",
   /* 20 */   "SIGTSTP",
   /* 21 */   "SIGTTIN",
   /* 22 */   "SIGTTOU",
   /* 23 */   "SIGURG",
   /* 24 */   "SIGXCPU",
   /* 25 */   "SIGXFSZ",
   /* 26 */   "SIGVTALRM",
   /* 27 */   "SIGPROF"
   /* 28 */   "SIGWINCH"
   /* 29 */   "SIGIO"
   /* 30 */   "SIGPWR"
   /* 31 */   "SIGSYS"
};

#define NUM_SIGNALS 32

static bool
signal_is_valid(const struct signal *s)
{
   if (s->num <= 0)
      return false;
   else if(s->num >= NUM_SIGNALS)
      return false;
   else
      return true;
}

static void
signal_print(const struct signal *s, struct string *dest)
{
   if (!signal_is_valid(s)) {
      string_append_format(dest, "Unkown signal %d", s->num);
      return;
   }

   char *signal_str = signal_to_str[s->num];
   string_append_cstring(dest, signal_str);
}

// Take from man 2 signal
//
//   struct sigaction {
//      void     (*sa_handler)(int);
//      void     (*sa_sigaction)(int, siginfo_t *, void *);
//      sigset_t   sa_mask;
//      int        sa_flags;
//      void     (*sa_restorer)(void);
//   };
//

struct global_signal_handler {
   struct signal_handler_vec handlers;
};
struct global_signal_handler global_signal_handler;

static struct signal_handler *
get_signal_handler(struct signal sig)
{
   return signal_handler_vec_at(&global_signal_handler.handlers, sig.num);
}

static void
handle_signal(int signum)
{
   struct signal sig = { signum };
   adt_assert(signal_is_valid(&sig),
      "Got unkown signal %s", print(signal, &sig));

   const struct signal_handler *handler = get_signal_handler(sig);

   adt_assert(handler->func != NULL,
      "Got signal %s unexpectedly", print(signal, &sig));

   handler->func();
}

static void
_set_signal_handler(int signum, void (*handle_signal)(int))
{
   struct sigaction sa;
   memset(&sa, 0, sizeof(struct sigaction));
   sa.sa_handler = handle_signal;
   int error = sigaction(signum, &sa, NULL);

   if (error != 0)
      ecrash_errno_error();
}

void
set_signal_handler(int signum, void (*func)())
{
   adt_assert(func != NULL, "NULL signal handling function given");
   if (signal_handler_vec_size(&global_signal_handler.handlers) == 0)
      signal_handler_vec_resize(&global_signal_handler.handlers, NUM_SIGNALS);

   struct signal sig = { signum };
   struct signal_handler *handler = get_signal_handler(sig);

   handler->func = func;

   _set_signal_handler(signum, handle_signal);
}

void
ignore_signal(int signum)
{
   struct signal sig = { signum };
   struct signal_handler *handler a_unused = get_signal_handler(sig);

   adt_assert(handler->func == NULL);

   _set_signal_handler(signum, SIG_IGN);
}
