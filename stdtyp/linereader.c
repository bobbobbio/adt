#include <stdtyp/linereader.h>
#include <stdtyp/stream.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

adt_func_body(line_reader);

#define BUFFER_SIZE 4096

void
line_reader_init(struct line_reader *l)
{
   string_init(&l->buff);
   l->start = 0;
   l->done = false;
   l->stream = NULL;
}

struct line_reader
line_reader_make_var(struct stream *stream)
{
   struct line_reader l = line_reader_make();
   line_reader_set_stream(&l, stream);
   return l;
}

void
line_reader_destroy(struct line_reader *l)
{
   string_destroy(&l->buff);
}

static struct error
line_reader_read(struct line_reader *l, bool *done)
{
   adt_assert(l->stream != NULL);

   ereraise(stream_read_n_or_less(l->stream, &l->buff, BUFFER_SIZE));

   if (string_length(&l->buff) == 0)
      l->done = true;

   l->start = 0;

   return no_error;
}

void
line_reader_set_stream(struct line_reader *l, struct stream *stream)
{
   adt_assert(l->stream == NULL);
   l->stream = stream;
}

bool
line_reader_get_line(struct line_reader *l, struct string *s)
{
   if (l->done)
      return false;

   if (l->start >= string_length(&l->buff)) {
      line_reader_read(l, &l->done);
      if (l->done)
         return false;
   }

   string_clear(s);
   uint64_t i = l->start;
   while (string_char_at_index(&l->buff, i) != '\n') {
      // XXX what do we do about \r????
      if (string_char_at_index(&l->buff, i) != '\r')
         string_append_char(s, string_char_at_index(&l->buff, i));
      i++;
      if (i >= string_length(&l->buff)) {
         line_reader_read(l, &l->done);
         if (l->done)
            break;
         i = l->start;
      }
   }
   l->start = i + 1;

   return true;
}
