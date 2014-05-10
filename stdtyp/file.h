#ifndef __FILE_H__
#define __FILE_H__

#include <adt.h>

#include <stdtyp/string.h>

void
fd_destroy(int *);

#define create_fd(name, val) \
   int name a_cleanup(fd_destroy) = val

create_error_header(file_write_error);
create_error_header(file_read_error);
create_error_header(file_not_found_error);
create_error_header(file_access_error);
create_error_header(file_out_of_space_error);
create_error_header(file_already_exists_error);
create_error_header(errno_unknown_error);

struct error
file_list_directory(const struct string *, struct string_vec *);

struct error
errno_to_error(void);

#endif // __FILE_H__
