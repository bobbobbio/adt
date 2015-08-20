#ifndef __CTXMANAGER_H__
#define __CTXMANAGER_H__

/*
 * This file provides a way to create custom context managers.
 *
 * Use it the following way:
 *
 * Say you have an object that is a global logger, when enabled all log
 * statements get written to it.  It has an interface like this:
 *
 * create(logger, l);
 * logger_start_logging(&l);
 * log("This gets logged to the logger named l");
 * logger_stop_logging(&l);
 * log("This log message gets lost")
 *
 * It would be handy to create a context manager for the logger, because
 * whenever you start it, you definitely want to stop it at some point. To do
 * that, do the following:
 *
 * In the header define:
 *
 * context_manager_gen_header(logger_cm, logger);
 * #define with_logger(logger) \
 *    with_context_manager(logger_cm, logger)
 *
 * In the c file define:
 *
 * context_manager_gen_body(logger_cm, logger,
 *    logger_start_logging, logger_stop_logging);
 *
 * The you can use the context manager like this:
 *
 * with_logger (&l) {
 *    log("This will get logger to the logger named l");
 * }
 *
 * It will take care of calling your start and stop functions, passing logger.
 *
 * Sometimes you want a context manager that creates an object for the duration
 * of the context, instead of passing it in. For that there is with_create_var.
 *
 * Here is an example for a context manager that opens a file.
 *
 * #define with_file_open(name, path, flags) \
 *    with_create_var(file, name, file_make_open, path, flags)
 *
 * The file_make_open function here creates a file with the following using the
 * arguments that follow it, and also opens the file.  If your create function
 * doesn't take any arugments use the with_create macro instead.  The destroy
 * for the file object with close the file automatically.
 *
 * TODO: A current limitation is that the enter, exit function must have return
 * type struct error, and have only the object pointer as arguments.
 * TODO: Another thing that would be nice, would be if the wth_context_manager
 * somehow could take arguments to pass to the enter function.
 *
 */

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
      ecrash(enter(t)); \
      return (struct name) { t }; \
   } \
   void \
   name##_context_manager_exit(struct name *n) { \
      ecrash(exit(n->v)); \
   } \
   SWALLOWSEMICOLON

#define context_manager_gen_static(name, type, enter, exit) \
   context_manager_gen_header(name, type); \
   context_manager_gen_body(name, type, enter, exit)

#define with_context_manager(name, t) \
   ctx_def(struct name a_unused unq(cm) \
      a_cleanup(name##_context_manager_exit) = \
      name##_context_manager_enter(t))

#define with_create_var(type, name, cfunc, ...) \
   ctx_def(struct type name a_cleanup(type##_destroy) = cfunc(__VA_ARGS__))

#define with_create(type, name) \
   ctx_def(struct type name a_cleanup(type##_destroy) = type##_make())

#endif // __CTXMANAGER_H__
