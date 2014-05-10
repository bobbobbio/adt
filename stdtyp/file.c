#include <stdtyp/file.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>

create_error_body(file_write_error);
create_error_body(file_read_error);
create_error_body(file_not_found_error);
create_error_body(file_access_error);
create_error_body(file_out_of_space_error);
create_error_body(file_already_exists_error);
create_error_body(errno_unknown_error);

void
fd_destroy(int *fd) {
   if (*fd != -1)
      close(*fd);
   *fd = -1;
}

struct error
errno_to_error(void)
{
   if (errno == EINTR)
      panic("temporary failure caused crash");
   else if (errno == EAGAIN)
      panic("temporary failure caused crash");

   if (errno == EFAULT)
      panic("Path pointed to invalid memory");
   else if (errno == EINVAL)
      panic("oflag was not set");

   if (errno == EACCES)
      return error_make(file_access_error, "Insufficent access");
   else if (errno == ENOENT)
      return error_make(file_not_found_error, "Directory not found");
   else if (errno == EEXIST)
      return error_make(file_already_exists_error, "File already exists");
   else if (errno == ENOSPC)
      return error_make(file_out_of_space_error, "File system out of space");
   else
      return error_make(errno_unknown_error, "");
}


// From the man page:
//
// struct dirent {
//    ino_t          d_ino;       /* inode number */
//    off_t          d_off;       /* not an offset; see NOTES */
//    unsigned short d_reclen;    /* length of this record */
//    unsigned char  d_type;      /* type of file; not supported
//                                   by all filesystem types */
//    char           d_name[256]; /* filename */
// };

struct error
file_list_directory(const struct string *path, struct string_vec *files_out)
{
   string_vec_clear(files_out);

   // Open the directory
   DIR *dirp = opendir(string_to_cstring(path));
   if (dirp == NULL) {
      closedir(dirp);
      return errno_to_error();
   }

   // Iterate over the files
   while (true) {
      struct dirent *dirent = readdir(dirp);
      if (dirent == NULL)
         break;

      create_string(file_name, dirent->d_name);
      if (string_equal(&file_name, strw(".")))
         continue;
      else if (string_equal(&file_name, strw("..")))
         continue;

      string_vec_append(files_out, &file_name);
   }

   if (closedir(dirp) == -1)
      return errno_to_error();

   return no_error;
}
