#include <adt.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdtyp/threading.h>
#include <unistd.h>

create_error_body(mutex_already_owned_error);
create_error_body(mutex_not_owned_error);

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

static void *
__thread_join(struct thread *thread)
{
   void *return_value;
   pthread_join(thread->t, &return_value);
   return return_value;
}

void
_thread_join(struct thread *thread, void *ret, int ret_size)
{
   void *return_value = __thread_join(thread);
   if (ret != NULL && return_value != NULL) {
      memcpy(ret, return_value, ret_size);
   }
   free(return_value);
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
      free(__thread_join(thread));
   thread_vec_clear(&tp->threads);
}

adt_func_body(mutex);

context_manager_gen_body(mutex_cm, mutex, mutex_lock, mutex_unlock);

void
mutex_init(struct mutex *m)
{
   pthread_mutex_init(&m->_lock, NULL);
}

void
mutex_destroy(struct mutex *m)
{
   pthread_mutex_destroy(&m->_lock);
}

struct error
mutex_lock(struct mutex *m)
{
   int error = pthread_mutex_lock(&m->_lock);

   assert_msg(error != EINVAL, "Lock was invalid");
   assert_msg(error != EAGAIN, "Too many recursive locks taken");

   if (error == EDEADLK)
      return error_make(mutex_already_owned_error, "");
   else {
      assert_msg(error == 0, "Unexpected error");
      return no_error;
   }
}

struct error
mutex_unlock(struct mutex *m)
{
   int error = pthread_mutex_unlock(&m->_lock);

   assert_msg(error != EINVAL, "Lock was invalid");
   assert_msg(error != EAGAIN, "Too many recursive locks taken");

   if (error == EPERM)
      return error_make(mutex_not_owned_error, "");
   else {
      assert_msg(error == 0, "Unexpected error");
      return no_error;
   }
}
