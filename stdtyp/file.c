// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <stdtyp/file.h>
#include <stdtyp/stream.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

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
      eraise_errno_error();
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
      eraise_errno_error();
   else
      f->fd = -1;

   return no_error;
}

int
file_fd(const struct file *f)
{
   return f->fd;
}

void
file_set_fd(struct file *f, int fd)
{
   f->fd = fd;
}

void
file_destroy(struct file *f)
{
   ecrash(file_close(f));
   f->done = true;
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
      eraise_errno_error();
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
      eraise_errno_error();

   return no_error;
}

// From the man page (man 2 stat):
//
// struct stat {
//    dev_t     st_dev;     /* ID of device containing file */
//    ino_t     st_ino;     /* inode number */
//    mode_t    st_mode;    /* protection */
//    nlink_t   st_nlink;   /* number of hard links */
//    uid_t     st_uid;     /* user ID of owner */
//    gid_t     st_gid;     /* group ID of owner */
//    dev_t     st_rdev;    /* device ID (if special file) */
//    off_t     st_size;    /* total size, in bytes */
//    blksize_t st_blksize; /* blocksize for filesystem I/O */
//    blkcnt_t  st_blocks;  /* number of 512B blocks allocated */
//    time_t    st_atime;   /* time of last access */
//    time_t    st_mtime;   /* time of last modification */
//    time_t    st_ctime;   /* time of last status change */
// };

static struct error
file_stat(const struct string *path, struct stat *stat_out)
{
   adt_assert(stat_out != NULL);
   int err = stat(string_to_cstring(path), stat_out);
   if (err < 0)
      eraise_errno_error();

   return no_error;
}

bool
path_is_dir(const struct string *path)
{
   struct stat res = {0};

   // Any error results in false
   ehandle(error, file_stat(path, &res))
      return false;

   return S_ISDIR(res.st_mode);
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
      eraise_errno_error();

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

   ereraise(string_read_fd(buff, fd_s->fd, want, got, &fd_s->done));

   return no_error;
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
            eraise_errno_error();
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
