#include <stdtyp/linereader.h>
#include <stdtyp/file.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

adt_func_body(line_reader);

#define BUFFER_SIZE 4096

void
line_reader_init(struct line_reader *l)
{
   l->fd = -1;
   l->buff = (char *)malloc(BUFFER_SIZE);
   l->start = 0;
   l->size = 0;
   l->done = false;
   l->should_close = false;
}

void
line_reader_destroy(struct line_reader *l)
{
   if (l->should_close)
      close(l->fd);
   free(l->buff);
}

static struct error
line_reader_read(struct line_reader *l, bool *done)
{
   assert(line_reader_opened(l));
   while (true) {
      l->size = read(l->fd, l->buff, BUFFER_SIZE);
      if (l->size == -1) {
         if (errno == EINTR || errno == EAGAIN)
            continue;
         else
            return errno_to_error();
      }
      break;
   }

   if (l->size == 0)
      *done = true;

   l->start = 0;

   return no_error;
}

struct error
line_reader_open_stdin(struct line_reader *l)
{
   l->fd = STDIN_FILENO;
   l->should_close = false;
   return no_error;
}

struct error
line_reader_open_fd(struct line_reader *l, int fd)
{
   l->fd = fd;
   l->should_close = false;
   return no_error;
}

struct error
line_reader_open_file(struct line_reader *l, const struct string *path)
{
   // open file
   l->fd = open(string_to_cstring(path), O_RDONLY);
   if (l->fd == -1)
      return errno_to_error();

   l->should_close = true;

   return no_error;
}

bool line_reader_get_line(struct line_reader *l, struct string *s)
{
   if (l->done)
      return false;

   if (l->start >= l->size) {
      echeck(line_reader_read(l, &l->done));
      if (l->done)
         return false;
   }

   string_clear(s);
   uint64_t i = l->start;
   while (l->buff[i] != '\n') {
      // XXX what do we do about \r????
      if (l->buff[i] != '\r')
         string_append_char(s, l->buff[i]);
      i++;
      if (i >= l->size) {
         echeck(line_reader_read(l, &l->done));
         if (l->done)
            break;
         i = l->start;
      }
   }
   l->start = i + 1;

   return true;
}

bool
line_reader_opened(const struct line_reader *l)
{
   return l->fd != -1;
}
