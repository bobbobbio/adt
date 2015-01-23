#include <stdtyp/stream.h>
#include <stdtyp/file.h>

#include <errno.h>
#include <unistd.h>

adt_func_body(fd_stream);

void
fd_stream_init(struct fd_stream *s)
{
   s->fd = -1;
   s->type = FD_STREAM;
   s->done = false;
}

void
fd_stream_destroy(struct fd_stream *s) {}

struct fd_stream
fd_stream_make_var(int fd) {
   struct fd_stream s = fd_stream_make();
   fd_stream_set_fd(&s, fd);
   return s;
}

void
fd_stream_set_fd(struct fd_stream *s, int fd)
{
   s->fd = fd;
}

struct error
fd_stream_read(struct stream *s, struct string *buff, size_t want, size_t *got)
{
   assert(s->type == FD_STREAM);
   struct fd_stream *fd_s = (struct fd_stream *)s;
   assert_msg(fd_s->fd != -1, "File descriptor not open");

   return string_read_fd(buff, fd_s->fd, want, got, &fd_s->done);
}

struct error
fd_stream_write(struct stream *s, struct string *data)
{
   assert(s->type == FD_STREAM);
   struct fd_stream *fd_s = (struct fd_stream *)s;
   assert_msg(fd_s->fd != -1, "File descriptor not open");

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
   assert(s->type == FD_STREAM);
   struct fd_stream *fd_s = (struct fd_stream *)s;
   assert_msg(fd_s->fd != -1, "File descriptor not open");

   return !fd_s->done;
}
