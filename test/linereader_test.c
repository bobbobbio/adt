#include <stdtyp/linereader.h>
#include <stdtyp/tokenizer.h>
#include <error.h>

int
main(int argc, char **argv)
{
   create(line_reader, lr);

   echeck(line_reader_open_file(&lr, strw("linereader_test.c")));

   create(string, line);
   while (line_reader_get_line(&lr, &line)) {
      create_tokenizer(tkn, &line);
      create(string, t);
      while (tokenizer_get_next(&tkn, &t)) {
         assert(string_length(&t) > 0);
      }
   }

   return EXIT_SUCCESS;
}
