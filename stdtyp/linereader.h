// Copyright 2015 Remi Bernotavicius. All right reserved.

#ifndef __LINE_READER_H__
#define __LINE_READER_H__

#include <stdtyp/string.h>

struct line_reader {
   struct string buff;
   uint64_t start;
   bool done;
   struct stream *stream;

   struct string iter_str;
};
adt_func_header(line_reader);

#define create_line_reader(name, stream) \
   struct line_reader name a_cleanup(line_reader_destroy) = \
      line_reader_make_var(stream)

struct line_reader
line_reader_make_var(struct stream *stream);

void
line_reader_set_stream(struct line_reader *, struct stream *);

struct error
line_reader_get_line(struct line_reader *, struct string *, bool *got_line)
   a_warn_unused_result;

iter_gen_header(line_reader, int, const struct string *);

#endif // __LINE_READER_H__
