#ifndef __THREADING_H__
#define __THREADING_H__

#include <stdtyp/vector.h>
#include <pthread.h>
#include <error.h>
#include <stdtyp/string.h>
#include <stdtyp/ctxmanager.h>

/*
 * This file provides utilities for working with threads, and multithreaded
 * applications.
 *
 * Functions that are run by threads need to be wrapped in a special function
 * pointer type.  Do that like this:
 *
 * void
 * my_func(void)
 * {
 *    printf("Another thread");
 * }
 * fptr_define_void_noargs(my_func);
 *
 * or like this:
 *
 * bool
 * equal(int a, int b)
 * {
 *    return a == b;
 * }
 * fptr_define(add, bool, int, int);
 *
 * Start a thread like this:
 *
 * struct thread *t = thread_run_noargs(my_func);
 *
 * or like this:
 *
 * struct thread *t = thread_run(equal, 4, 5);
 *
 * When you join the thread, it gives you the value returned from the fptr.  If
 * the thread function is void return type, just pass NULL for the second
 * argument to thread_join.It will also free the thread pointer, (and is the
 * only way to do so)
 *
 * int result;
 * thread_join(t, &result);
 *
 * When working with multiple thread, use the thread pool type.
 *
 * create(thread_pool, tp);
 *
 * thread_pool_run(&tp, equal, 2, 3);
 * thread_pool_run(&tp, equal, 4, 5);
 * thread_pool_run(&tp, equal, 6, 9);
 *
 * thread_pool_join(&tp);
 *
 * The thread pool doesn't yet support getting the return values of the threads.
 *
 * There is a mutex type for locking:
 *
 * create(mutex, m);
 *
 * mutex_lock(&m);
 * // protected by the lock
 * mutex_unlock(&m);
 *
 * There is also a context manager:
 *
 * with_mutex (&m) {
 *    // this is also protected
 * }
 *
 */

struct thread;

vector_gen_header(thread_vec, thread);

struct thread_pool {
   struct thread_vec threads;
};
adt_func_header(thread_pool);

#define mkvar(t, v) \
   t v;

#define _struct_make(name, a, b, c, d, e, _a, _b, _c, _d, _e, \
   __a, __b, __c, __d, __e, ...) \
   struct name { \
      __a(a, a_) \
      __b(b, b_) \
      __c(c, c_) \
      __d(d, c_) \
      __e(e, c_) \
   }

#define struct_make(...) \
   _struct_make(__VA_ARGS__, , , , , , \
      mkvar, mkvar, mkvar, mkvar, mkvar, noop, noop, noop, noop, noop)

#define cm(x) \
   x,

#define _fptr_call(func, args, a, b, c, d, e, _a, _b, _c, _d, _e, ...) \
   func( \
   _a \
   _b \
   _c \
   _d \
   _e)

#define fptr_call(func, args, ...) \
   _fptr_call(func, args, __VA_ARGS__, \
      cm(args->e), cm(args->d), cm(args->c), cm(args->b), args->a_, \
      , , , , )

#define fptr_run(func, ...) \
   func##_stub, sizeof(struct func##_args), &(struct func##_args){ __VA_ARGS__ }

#define fptr_run_noargs(func) \
   func##_stub, 0, NULL

#define thread_pool_run(tpool, ...) \
   _thread_pool_run(tpool, fptr_run(__VA_ARGS__))

#define thread_run(...) \
   _thread_run(fptr_run(__VA_ARGS__))

#define thread_pool_run_noargs(tpool, func) \
   _thread_pool_run(tpool, fptr_run_noargs(func))

#define thread_run_noargs(func) \
   _thread_run(fptr_run_noargs(func))

#define fptr_define(func, ret, ...) \
   struct_make(func##_args, __VA_ARGS__); \
   void * \
   func##_stub(void *_args) \
   { \
      struct func##_args *args = _args; \
      ret *v = malloc(sizeof(ret)); \
      *v = fptr_call(func, args, __VA_ARGS__); \
      free(args); \
      pthread_exit(v); \
   } \
   SWALLOWSEMICOLON

#define fptr_define_noargs(func, ret) \
   void * \
   func##_stub(void *_args) \
   { \
      ret *v = malloc(sizeof(ret)); \
      *v = func(); \
      pthread_exit(v); \
   } \
   SWALLOWSEMICOLON

#define fptr_define_void(func, ...) \
   struct_make(func##_args, __VA_ARGS__); \
   void * \
   func##_stub(void *_args) \
   { \
      struct func##_args *args = _args; \
      fptr_call(func, args, __VA_ARGS__); \
      free(args); \
      pthread_exit(NULL); \
   } \
   SWALLOWSEMICOLON

#define fptr_define_void_noargs(func) \
   void * \
   func##_stub(void *_args) \
   { \
      func(); \
      pthread_exit(NULL); \
   } \
   SWALLOWSEMICOLON

struct thread *
_thread_run(void *(*func)(void *), int args_size, void *args);

void
_thread_pool_run(struct thread_pool *tpool, void *(*func)(void *),
   int args_size, void *args);

void
thread_pool_join(struct thread_pool *tp);

void
_thread_join(struct thread *, void *ret, int ret_size);

#define thread_join(thread, ret) \
   _thread_join(thread, ret, sizeof(*ret))

#define thread_join_void(thread) \
   _thread_join(thread, NULL, 0)

struct mutex {
   pthread_mutex_t _lock;
};
adt_func_header(mutex);

// The current thread already owns the lock
create_error_header(mutex_already_owned_error);

// Tried to unlock but calling thread doesn't own lock
create_error_header(mutex_not_owned_error);

struct error
mutex_lock(struct mutex *m);

struct error
mutex_unlock(struct mutex *m);

context_manager_gen_header(mutex_cm, mutex);

#define with_mutex(mutex) \
   with_context_manager(mutex_cm, mutex)

#endif // __THREADING_H__
