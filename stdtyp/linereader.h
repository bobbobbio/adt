#include <stdtyp/string.h>

struct line_reader {
   int fd;
   char *buff;
   uint64_t start;
   uint64_t size;
   bool done;
};
adt_func_header(line_reader);

struct error
line_reader_open_file(struct line_reader *, const struct string *);

bool
line_reader_get_line(struct line_reader *, struct string *);
