#include <stdtyp/file.h>
#include <stdtyp/stream.h>

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
   f->type = FD_STREAM;
   f->fd = -1;
   f->done = false;
}

void
file_copy(struct file *d, const struct file *s)
{
   *d = *s;
}

struct error
file_open(struct file *f, const struct string *path, int flags)
{
   int fd = open(string_to_cstring(path), flags);
   if (fd == -1)
      return errno_to_error();
   else {
      f->fd = fd;
      return no_error;
   }
}

struct file
file_make_open(const struct string *path, int flags)
{
   struct file file;
   file_init(&file);
   ecrash(file_open(&file, path, flags));
   return file;
}

struct error
file_close(struct file *f)
{
   if (f->fd == -1)
      return no_error;

   if (close(f->fd) == -1)
      ereraise(errno_to_error());
   else {
      f->fd = -1;
   }

   return no_error;
}

int
file_fd(const struct file *f)
{
   return f->fd;
}

void
file_destroy(struct file *f)
{
   ecrash(file_close(f));
   f->fd = -1;
   f->done = true;
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
      eraise(file_access_error, strerror(errno));
   else if (errno == ENOENT)
      eraise(file_not_found_error, strerror(errno));
   else if (errno == EEXIST)
      eraise(file_already_exists_error, strerror(errno));
   else if (errno == ENOSPC)
      eraise(file_out_of_space_error, strerror(errno));
   else
      eraise(errno_unknown_error, strerror(errno));
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

/*  __ _ _                 _
 * / _(_) | ___   ___  ___| |_
 * | |_| | |/ _ \ / __|/ _ \ __|
 * |  _| | |  __/ \__ \  __/ |_
 * |_| |_|_|\___| |___/\___|\__|
 *
 */

adt_func_pod_body(file_set);

struct file_set
file_set_make_var(const struct file *a, ...)
{
   va_list argp;
   va_start(argp, a);

   struct file_set out = file_set_make();

   while (a != NULL) {
      file_set_set(&out, a);
      a = va_arg(argp, const struct file *);
   }

   return out;
}

void
file_set_zero(struct file_set *f)
{
   FD_ZERO(&f->fds);
}

void
file_set_set(struct file_set *f, const struct file *file)
{
   if (file_fd(file) > f->highest_fd)
      f->highest_fd = file_fd(file);

   FD_SET(file_fd(file), &f->fds);
}

struct error
file_set_select(struct file_set *f)
{
   int err = select(f->highest_fd + 1, &f->fds, NULL, NULL, NULL);

   if (err < 0)
      ereraise(errno_to_error());

   return no_error;
}

bool
file_set_is_set(struct file_set *f, const struct file *file)
{
   return FD_ISSET(file_fd(file), &f->fds);
}

/*   __ _ _            _
 *  / _(_) | ___   ___| |_ _ __ ___  __ _ _ __ ___
 * | |_| | |/ _ \ / __| __| '__/ _ \/ _` | '_ ` _ \
 * |  _| | |  __/ \__ \ |_| | |  __/ (_| | | | | | |
 * |_| |_|_|\___| |___/\__|_|  \___|\__,_|_| |_| |_|
 */

const struct stream_interface fd_stream_interface = {
   .stream_read = fd_stream_read,
   .stream_write = fd_stream_write,
   .stream_has_more = fd_stream_has_more
};

struct stream *
file_to_stream(struct file *f)
{
   return (struct stream *)f;
}

struct error
fd_stream_read(struct stream *s, struct string *buff, size_t want, size_t *got)
{
   adt_assert(s->type == FD_STREAM);
   struct file *fd_s = (struct file *)s;
   adt_assert(fd_s->fd != -1, "File descriptor not open");

   return string_read_fd(buff, fd_s->fd, want, got, &fd_s->done);
}

struct error
fd_stream_write(struct stream *s, const struct string *data)
{
   adt_assert(s->type == FD_STREAM);
   struct file *fd_s = (struct file *)s;
   adt_assert(fd_s->fd != -1, "File descriptor not open");

   unsigned to_write = string_length(data);
   const char *d = string_to_cstring(data);

   while (to_write > 0) {
      int written = write(
         fd_s->fd, &d[string_length(data) - to_write], to_write);
      if (written == -1) {
         if (errno == EINTR || errno == EAGAIN)
            continue;
         else
            return errno_to_error();
      }
      to_write -= written;
   }

   return no_error;
}

bool
fd_stream_has_more(struct stream *s)
{
   adt_assert(s->type == FD_STREAM);
   struct file *fd_s = (struct file *)s;
   adt_assert(fd_s->fd != -1, "File descriptor not open");

   return !fd_s->done;
}
