#ifndef __CTXMANAGER_H__
#define __CTXMANAGER_H__

#define context_manager_gen_header(name, type) \
   struct name { \
      struct type *v; \
   }; \
   struct name \
   name##_context_manager_enter(struct type *); \
   void \
   name##_context_manager_exit(struct name *); \
   SWALLOWSEMICOLON

#define context_manager_gen_body(name, type, enter, exit) \
   struct name \
   name##_context_manager_enter(struct type *t) { \
      echeck(enter(t)); \
      return (struct name) { t }; \
   } \
   void \
   name##_context_manager_exit(struct name *n) { \
      echeck(exit(n->v)); \
   } \
   SWALLOWSEMICOLON

#define context_manager_gen_static(name, type, enter, exit) \
   context_manager_gen_header(name, type); \
   context_manager_gen_body(name, type, enter, exit)

#define with_context_manager(name, t) \
   ctx_def(struct name unq(cm) a_cleanup(name##_context_manager_exit) = \
      name##_context_manager_enter(t))

#define with_create_var(type, name, cfunc, ...) \
   ctx_def(struct type name a_cleanup(type##_destroy) = cfunc(__VA_ARGS__))

#define with_create(type, name) \
   ctx_def(struct type name a_cleanup(type##_destroy) = type##_make())

#endif // __CTXMANAGER_H__
