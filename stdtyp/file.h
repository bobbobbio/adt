#ifndef __FILE_H__
#define __FILE_H__

#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>

#include <adt.h>

#include <stdtyp/string.h>
#include <stdtyp/ctxmanager.h>
#include <stdtyp/stream.h>

create_error_header(file_not_found_error);
create_error_header(file_access_error);
create_error_header(file_out_of_space_error);
create_error_header(file_already_exists_error);
create_error_header(errno_unknown_error);

struct file {
   struct fd_stream stream;
};
adt_func_header(file);

#define create_file_fd(name, fd) \
   struct file name a_cleanup(file_destroy) = { FD_STREAM, fd };

#define fd_wrap(fd_name) \
   (&(struct file){ (struct fd_stream){.fd = fd_name, .type = FD_STREAM } })
#define file_stdin fd_wrap(STDIN_FILENO)
#define file_stdout fd_wrap(STDOUT_FILENO)
#define file_stderr fd_wrap(STDERR_FILENO)

struct error
file_open(struct file *, const struct string *path, int flags)
   a_warn_unused_result;

struct file
file_make_open(const struct string *path, int flags);

struct error
file_close(struct file *f)
   a_warn_unused_result;

int
file_fd(const struct file *f);

struct error
file_list_directory(const struct string *, struct string_vec *)
   a_warn_unused_result;

struct error
errno_to_error(void)
   a_warn_unused_result;

#define with_file_open(name, path, flags) \
   with_create_var(file, name, file_make_open, path, flags)

struct file_set {
   fd_set fds;
   int highest_fd;
};
adt_func_pod_header(file_set);

#define create_file_set(name, ...) \
   struct file_set name a_cleanup(file_set_destroy) = \
      file_set_make_var(__VA_ARGS__, NULL)

struct file_set
file_set_make_var(const struct file *a, ...);

void
file_set_zero(struct file_set *);

void
file_set_set(struct file_set *, const struct file *);

struct error
file_set_select(struct file_set *)
   a_warn_unused_result;

bool
file_set_is_set(struct file_set *, const struct file *);

#endif // __FILE_H__
