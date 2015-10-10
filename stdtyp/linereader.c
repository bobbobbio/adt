// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <stdtyp/linereader.h>
#include <stdtyp/stream.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

adt_func_body(line_reader);

#define BUFFER_SIZE 4096

iter_gen_body(line_reader, int, const struct string *);

bool line_reader_iter_next(
   const struct line_reader *const_self,
   struct line_reader_iter *i)
{
   struct line_reader *self = (struct line_reader *)const_self;
   i->key++;
   bool got_line = false;
   ecrash(line_reader_get_line(self, &self->iter_str, &got_line));
   i->value = &self->iter_str;
   return got_line;
}

void
line_reader_init(struct line_reader *l)
{
   string_init(&l->buff);
   l->start = 0;
   l->done = false;
   l->stream = NULL;
   string_init(&l->iter_str);
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
   string_init(&l->iter_str);
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

struct error
line_reader_get_line(struct line_reader *l, struct string *s, bool *got_line)
{
   bool dummy;
   if (got_line == NULL)
      got_line = &dummy;

   if (l->done) {
      *got_line = false;
      return no_error;
   }

   if (l->start >= string_length(&l->buff)) {
      ereraise(line_reader_read(l, &l->done));
      if (l->done) {
         *got_line = false;
         return no_error;
      }
   }

   string_clear(s);
   uint64_t i = l->start;
   while (string_char_at_index(&l->buff, i) != '\n') {
      // XXX what do we do about \r????
      if (string_char_at_index(&l->buff, i) != '\r')
         string_append_char(s, string_char_at_index(&l->buff, i));
      i++;
      if (i >= string_length(&l->buff)) {
         ereraise(line_reader_read(l, &l->done));
         if (l->done)
            break;
         i = l->start;
      }
   }
   l->start = i + 1;

   *got_line = true;
   return no_error;
}
