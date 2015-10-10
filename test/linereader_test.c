// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <stdtyp/linereader.h>
#include <stdtyp/tokenizer.h>
#include <stdtyp/file.h>
#include <stdtyp/string_stream.h>
#include <error.h>
#include "test.h"

adt_test(linereader_test)
{
   int tokens = 0;
   with_file_open (file, strw("linereader_test.c"), 0) {
      create_line_reader(lr, file_to_stream(&file));

      iter_value (line_reader, &lr, line) {
         create_string_stream_const(ss, line);
         create_tokenizer(tkn, string_stream_to_stream(&ss));
         create(string, t);
         while (tokenizer_get_next(&tkn, &t)) {
            tokens++;
            aprintf("%s, ", print(string, &t));
            adt_assert(string_length(&t) > 0);
         }
      }
   }

   adt_assert(tokens > 10);
}

adt_test(tokenizer)
{
   create_string_stream_const(ss, strw("this is a sentence"));
   adt_assert(stream_has_more(string_stream_to_stream(&ss)));

   create_tokenizer(tkn, string_stream_to_stream(&ss));
   create(string, token);

   adt_assert(tokenizer_get_next(&tkn, &token));
   adt_assert(string_equal(&token, strw("this")));

   adt_assert(tokenizer_get_next(&tkn, &token));
   adt_assert(string_equal(&token, strw("is")));

   adt_assert(tokenizer_get_next(&tkn, &token));
   adt_assert(string_equal(&token, strw("a")));

   adt_assert(tokenizer_get_next(&tkn, &token));
   adt_assert(string_equal(&token, strw("sentence")));

   adt_assert(!tokenizer_get_next(&tkn, &token));
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
