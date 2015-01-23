#include <stdtyp/file.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>

create_error_body(file_not_found_error);
create_error_body(file_access_error);
create_error_body(file_out_of_space_error);
create_error_body(file_already_exists_error);
create_error_body(errno_unknown_error);

adt_func_body(file);

void
file_init(struct file *f)
{
   fd_stream_init(&f->stream);
}

struct error
file_open(struct file * f, const struct string *path, int flags)
{
   int fd = open(string_to_cstring(path), flags);
   if (fd == -1)
      return errno_to_error();
   else {
      fd_stream_set_fd(&f->stream, fd);
      return no_error;
   }
}

struct file
file_make_open(const struct string *path, int flags)
{
   struct file file;
   file_init(&file);
   echeck(file_open(&file, path, flags));
   return file;
}

struct error
file_close(struct file *f)
{
   if (f->stream.fd == -1)
      return no_error;

   if (close(f->stream.fd) == -1)
      return errno_to_error();
   else {
      f->stream.fd = -1;
      return no_error;
   }
}

int
file_fd(struct file *f)
{
   return f->stream.fd;
}

void
file_destroy(struct file *f)
{
   file_close(f);
   fd_stream_destroy(&f->stream);
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
      return error_make(file_access_error, strerror(errno));
   else if (errno == ENOENT)
      return error_make(file_not_found_error, strerror(errno));
   else if (errno == EEXIST)
      return error_make(file_already_exists_error, strerror(errno));
   else if (errno == ENOSPC)
      return error_make(file_out_of_space_error, strerror(errno));
   else
      return error_make(errno_unknown_error, strerror(errno));
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
