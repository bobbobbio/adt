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
   string_init(&l->buff);
   l->start = 0;
   l->done = false;
   l->file = NULL;
}

struct line_reader
line_reader_make_var(struct file *file)
{
   struct line_reader l = line_reader_make();
   line_reader_set_file(&l, file);
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
   assert(l->file != NULL);

   size_t bytes_read;
   epass(file_read_n_or_less(l->file, &l->buff, BUFFER_SIZE, &bytes_read));

   if (string_length(&l->buff) == 0)
      l->done = true;

   l->start = 0;

   return no_error;
}

void
line_reader_set_file(struct line_reader *l, struct file *file)
{
   assert(l->file == NULL);
   l->file = file;
}

bool
line_reader_get_line(struct line_reader *l, struct string *s)
{
   if (l->done)
      return false;

   if (l->start >= string_length(&l->buff)) {
      echeck(line_reader_read(l, &l->done));
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
         echeck(line_reader_read(l, &l->done));
         if (l->done)
            break;
         i = l->start;
      }
   }
   l->start = i + 1;

   return true;
}
