#ifndef __LINE_READER_H__
#define __LINE_READER_H__

#include <stdtyp/string.h>

struct line_reader {
   struct string buff;
   uint64_t start;
   bool done;
   struct file *file;
};
adt_func_header(line_reader);

#define create_line_reader(name, file) \
   struct line_reader name a_cleanup(line_reader_destroy) = \
      line_reader_make_var(file)

struct line_reader
line_reader_make_var(struct file *file);

void
line_reader_set_file(struct line_reader *, struct file *);

bool
line_reader_get_line(struct line_reader *, struct string *);

#endif // __LINE_READER_H__
