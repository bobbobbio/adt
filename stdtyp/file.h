// Copyright 2015 Remi Bernotavicius. All right reserved.

#ifndef __FILE_H__
#define __FILE_H__

#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>

#include <adt.h>

#include <stdtyp/string.h>
#include <stdtyp/ctxmanager.h>
#include <stdtyp/stream.h>

struct file {
   enum stream_type type;
   int fd;
   bool done;
} a_packed;
adt_func_header(file);

#define create_file_fd(name, fd) \
   struct file name a_cleanup(file_destroy) = { FD_STREAM, fd, false };

#define fd_wrap(fd_name) \
   (&(struct file){ FD_STREAM, fd_name, false })
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

void
file_set_fd(struct file *f, int fd);

struct error
file_list_directory(const struct string *, struct string_vec *)
   a_warn_unused_result;

// XXX: This doesn't really belong here
bool
path_is_dir(const struct string *)
    a_warn_unused_result;

#define with_file_open(name, path, flags) \
   with_create_var(file, name, file_make_open, path, flags)

#define file_read(file, ...) \
   stream_read(file_to_stream(file), __VA_ARGS__)

#define file_read_n(file, ...) \
   stream_read_n(file_to_stream(file), __VA_ARGS__)

#define file_read_n_or_less(file, ...) \
   stream_read_n_or_less(file_to_stream(file), __VA_ARGS__)

#define file_write(file, ...) \
   stream_write(file_to_stream(file), __VA_ARGS__)

#define file_has_more(file) \
   stream_has_more(file_to_stream(file))

/*  __ _ _                 _
 * / _(_) | ___   ___  ___| |_
 * | |_| | |/ _ \ / __|/ _ \ __|
 * |  _| | |  __/ \__ \  __/ |_
 * |_| |_|_|\___| |___/\___|\__|
 *
 */

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

/*   __ _ _            _
 *  / _(_) | ___   ___| |_ _ __ ___  __ _ _ __ ___
 * | |_| | |/ _ \ / __| __| '__/ _ \/ _` | '_ ` _ \
 * |  _| | |  __/ \__ \ |_| | |  __/ (_| | | | | | |
 * |_| |_|_|\___| |___/\__|_|  \___|\__,_|_| |_| |_|
 */

struct stream *
file_to_stream(struct file *f);

struct error
fd_stream_read(struct stream *, struct string *, size_t want, size_t *got)
   a_warn_unused_result;

struct error
fd_stream_write(struct stream *, const struct string *)
   a_warn_unused_result;

bool
fd_stream_has_more(struct stream *);

#endif // __FILE_H__
