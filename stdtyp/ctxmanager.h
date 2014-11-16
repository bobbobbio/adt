#ifndef __CTXMANAGER_H__
#define __CTXMANAGER_H__

#define context_manager_gen_header(name, type) \
   struct name { \
      struct type *v; \
   }; \
   struct name \
   name##_context_manager_enter(struct type *t); \
   void \
   name##_context_manager_exit(struct name *n); \
   SWALLOWSEMICOLON

#define context_manager_gen_body(name, type, enter, exit) \
   struct name \
   name##_context_manager_enter(struct type *t) { \
      enter(t); \
      return (struct name) { t }; \
   } \
   void \
   name##_context_manager_exit(struct name *n) { \
      exit(n->v); \
   } \
   SWALLOWSEMICOLON

#define context_manager_gen_static(name, type, enter, exit) \
   context_manager_gen_header(name, type); \
   context_manager_gen_body(name, type, enter, exit)

#define with_context_manager(name, t) \
   ctx_def(struct name unq(cm) a_cleanup(name##_context_manager_exit) = \
      name##_context_manager_enter(t))

#endif // __CTXMANAGER_H__
