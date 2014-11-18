#ifndef __FILE_H__
#define __FILE_H__

#include <adt.h>

#include <stdtyp/string.h>
#include <stdtyp/ctxmanager.h>

create_error_header(file_write_error);
create_error_header(file_read_error);
create_error_header(file_not_found_error);
create_error_header(file_access_error);
create_error_header(file_out_of_space_error);
create_error_header(file_already_exists_error);
create_error_header(errno_unknown_error);

struct file {
   int fd;
};
adt_func_header(file);

#define create_file_fd(name, fd) \
   struct file name a_cleanup(file_destroy) = { fd };

struct error
file_read(struct file *f, struct string *buff);

struct error
file_read_n(struct file *f, struct string *buff, size_t len);

struct error
file_write(struct file *f, struct string *);

struct error
file_open(struct file *, const struct string *path, int flags);

struct file
file_open_cm(const struct string *path, int flags);

struct error
file_close(struct file *f);

struct error
file_list_directory(const struct string *, struct string_vec *);

struct error
errno_to_error(void);

context_manager_gen_header(file_cm, file);

#define with_file_open(name, path, flags) \
   ctx_def(struct file name = file_open_cm(path, flags)) \
   with_context_manager(file_cm, &name) \

#endif // __FILE_H__
