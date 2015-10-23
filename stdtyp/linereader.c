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
   ecrash(line_reader_get_line(self, &i->value));
   return i->value != NULL;
}

void
line_reader_init(struct line_reader *l)
{
   string_init(&l->buff);
   string_init(&l->ebuff);
   // XXX: This is really lame
   memset(&l->iter_str, 0, sizeof(struct string));
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
   string_destroy(&l->ebuff);
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
line_reader_get_line(struct line_reader *l, const struct string **s)
{
   if (l->done) {
      *s = NULL;
      return no_error;
   }

   if (l->start >= string_length(&l->buff)) {
      ereraise(line_reader_read(l, &l->done));
      if (l->done) {
         *s = NULL;
         return no_error;
      }
   }

   string_clear(&l->ebuff);

   uint64_t i = l->start;
   while (string_char_at_index(&l->buff, i) != '\n') {
      i++;
      if (i >= string_length(&l->buff)) {
         string_append_cstring_length(&l->ebuff,
            &string_to_cstring(&l->buff)[l->start],
            string_length(&l->buff) - l->start);

         ereraise(line_reader_read(l, &l->done));
         i = l->start;
         if (l->done)
            break;
      }
   }

   if (string_length(&l->ebuff) > 0) {
      // If we had to put stuff into the extended buffer, we have to use that.
      string_append_cstring_length(&l->ebuff,
         &string_to_cstring(&l->buff)[l->start],
         i - l->start);
      *s = &l->ebuff;
   } else {
      // Otherwise, we construct a string that is pointing to inside of the
      // regular buffer.
      l->iter_str.buff = (char *)&string_to_cstring(&l->buff)[l->start];
      l->iter_str.buff_len = i - l->start;
      l->iter_str.length = l->iter_str.buff_len;
      l->iter_str.buff[l->iter_str.length] = '\0';
      *s = &l->iter_str;
   }

   l->start = i + 1;

   return no_error;
}
