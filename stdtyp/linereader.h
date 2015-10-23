// Copyright 2015 Remi Bernotavicius. All right reserved.

#ifndef __LINE_READER_H__
#define __LINE_READER_H__

#include <stdtyp/string.h>

struct line_reader {
   uint64_t start;
   bool done;
   struct stream *stream;

   // This is the buffer we read off of the stream into
   struct string buff;
   // When a line ends up spaning read boundary, the hold over gets put into
   // this string
   struct string ebuff;
   // This the struct string that we use to return a string out of get_line
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
line_reader_get_line(struct line_reader *, const struct string **)
   a_warn_unused_result;

iter_gen_header(line_reader, int, const struct string *);

#endif // __LINE_READER_H__
