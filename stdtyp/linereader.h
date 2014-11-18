#include <stdtyp/string.h>

struct line_reader {
   struct string buff;
   uint64_t start;
   bool done;
   struct file *file;
};
adt_func_header(line_reader);

void
line_reader_open(struct line_reader *, struct file *);

bool
line_reader_get_line(struct line_reader *, struct string *);

bool
line_reader_opened(const struct line_reader *);
