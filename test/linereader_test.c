#include <stdtyp/linereader.h>
#include <stdtyp/tokenizer.h>
#include <stdtyp/file.h>
#include <error.h>

int
main(int argc, char **argv)
{
   int tokens = 0;
   with_file_open (file, strw("linereader_test.c"), 0) {
      create_line_reader(lr, (struct stream *)&file);

      create(string, line);
      while (line_reader_get_line(&lr, &line)) {
         create_string_stream(ss, &line);
         create_tokenizer(tkn, (struct stream *)&ss);
         create(string, t);
         while (tokenizer_get_next(&tkn, &t)) {
            tokens++;
            aprintf("%s, ", print(string, &t));
            assert(string_length(&t) > 0);
         }
      }
   }

   assert(tokens > 10);

   create_string_stream(ss, (struct string *)strw("this is a sentence"));
   assert(stream_has_more((struct stream *)&ss));

   create_tokenizer(tkn, (struct stream *)&ss);
   create(string, token);

   assert(tokenizer_get_next(&tkn, &token));
   assert(string_equal(&token, strw("this")));

   assert(tokenizer_get_next(&tkn, &token));
   assert(string_equal(&token, strw("is")));

   assert(tokenizer_get_next(&tkn, &token));
   assert(string_equal(&token, strw("a")));

   assert(tokenizer_get_next(&tkn, &token));
   assert(string_equal(&token, strw("sentence")));

   assert(!tokenizer_get_next(&tkn, &token));

   return EXIT_SUCCESS;
}
