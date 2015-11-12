// Copyright 2015 Remi Bernotavicius. All right reserved.

// ptrace on OS X is a complete joke
#ifndef __APPLE__

#ifndef __PTRACE_H__
#define __PTRACE_H__

#include <adt.h>
#include <stdtyp/map.h>

struct syscall {
   unsigned num;
   unsigned retval;
   struct string path;
};
adt_func_header(syscall);

struct ptracer_child {
   pid_t pid;
   struct syscall *syscall;
};
adt_func_pod_header(ptracer_child);

map_gen_podk_header(pid_map, pid_t, ptracer_child);

struct ptracer {
   struct pid_map children;
};
adt_func_header(ptracer);

void
ptracer_trace(struct ptracer *self);

struct error
ptracer_subprocess(struct ptracer *self, const struct string_vec *cmd)
   a_warn_unused_result;


#endif // __PTRACE_H__
#endif // __APPLE__
