#include <stdtyp/linereader.h>
#include <stdtyp/tokenizer.h>
#include <stdtyp/file.h>
#include <error.h>

int
main(int argc, char **argv)
{
   int tokens = 0;
   with_file_open (file, strw("linereader_test.c"), 0) {
      create_line_reader(lr, &file);

      create(string, line);
      while (line_reader_get_line(&lr, &line)) {
         create_tokenizer(tkn, &line);
         create(string, t);
         while (tokenizer_get_next(&tkn, &t)) {
            tokens++;
            aprintf("%s, ", print(string, &t));
            assert(string_length(&t) > 0);
         }
      }
   }

   assert(tokens > 10);

   return EXIT_SUCCESS;
}
