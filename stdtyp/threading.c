#include <adt.h>

#include <stdtyp/threading.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>

struct thread {
   pthread_t t;
};
adt_func_pod_static(thread);

vector_gen_body(thread_vec, thread);

adt_func_body(thread_pool);

void
thread_print(const struct thread *t, struct string *s)
{
}

void
thread_pool_init(struct thread_pool *tp)
{
   thread_vec_init(&tp->threads);
}

void
thread_pool_destroy(struct thread_pool *tp)
{
   thread_pool_join(tp);
   thread_vec_destroy(&tp->threads);
}

static void
_thread_join(struct thread *thread)
{
   void *return_value;
   pthread_join(thread->t, &return_value);
}

void
thread_join(struct thread *thread)
{
   _thread_join(thread);
   thread_free(thread);
}

static void
__thread_run(struct thread *thread, void *(*func)(void *),
   int args_size, void *_args)
{
   void *args = NULL;
   if (args_size > 0) {
      args = malloc(args_size);
      memcpy(args, _args, args_size);
   }

   while (pthread_create(&thread->t, NULL, func, args)) {
      if (errno == EAGAIN || errno == EINTR)
         continue;
      else
         panic("Failed to create new thread");
   }
}

struct thread *
_thread_run(void *(*func)(void *), int args_size, void *args)
{
   struct thread *thread = thread_new();
   __thread_run(thread, func, args_size, args);
   return thread;
}

void
_thread_pool_run(struct thread_pool *tpool, void *(*func)(void *),
   int args_size, void *args)
{
   struct thread *thread = thread_vec_grow(&tpool->threads);
   __thread_run(thread, func, args_size, args);
}

void
thread_pool_join(struct thread_pool *tp)
{
   iter_value (thread_vec, &tp->threads, thread)
      _thread_join(thread);
   thread_vec_clear(&tp->threads);
}

