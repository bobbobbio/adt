#ifndef __THREADING_H__
#define __THREADING_H__

#include <stdtyp/vector.h>
#include <pthread.h>
#include <error.h>
#include <stdtyp/string.h>

struct thread;

vector_gen_header(thread_vec, thread);

struct thread_pool {
   struct thread_vec threads;
};
adt_func_header(thread_pool);

#define mkvar(t, v) \
   t v;

#define noop(...)

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

#endif // __THREADING_H__
