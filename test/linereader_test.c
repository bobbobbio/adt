// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <stdtyp/linereader.h>
#include <stdtyp/file.h>
#include <stdtyp/string_stream.h>
#include <error.h>
#include "test.h"

adt_test(linereader_test)
{
   int lines = 0;
   with_file_open (file, strw("linereader_test.c"), 0) {
      create_line_reader(lr, file_to_stream(&file));

      iter_value (line_reader, &lr, line) {
         create_string_stream_const(ss, line);
      }
   }

   adt_assert(lines > 10);
}

adt_test(line_reader_string_stream)
{
   create_string_vec(expected_vec, "aaa", "bbb", "ccc", "ddd");

   create_string_stream_const(str, strw("aaa\nbbb\nccc\nddd"));
   create_line_reader(lr, string_stream_to_stream(&str));
   iter_value (line_reader, &lr, line) {
      adt_assert_equal(string, string_vec_at(&expected_vec, 0), line);
      string_vec_remove(&expected_vec, 0);
   }

   adt_assert(string_vec_size(&expected_vec) == 0);
}
