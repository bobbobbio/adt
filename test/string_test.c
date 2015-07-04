#include <adt.h>
#include <stdtyp/string.h>
#include <stdtyp/regex.h>

int
main(int argc, char **argv)
{
   create_string(str, "FORMAT: ");
   string_append_format(&str, "%d, %d, %s", 1, 2, "THREE");
   adt_assert(string_equal(&str, strw("FORMAT: 1, 2, THREE")));

   create_string(str2, "nothing apple cat apple banana");

   create_regex(reg, strw("apple"));
   string_replace(&str2, &reg, strw("steve jobs"));

   adt_assert(string_equal(&str2,
      strw("nothing steve jobs cat steve jobs banana")));

   create_regex(reg2, strw("(a)"));
   string_replace(&str2, &reg2, strw("$1=[$1]\\$1"));

   adt_assert(string_equal(&str2,
      strw("nothing steve jobs ca=[a]$1t steve jobs "
      "ba=[a]$1na=[a]$1na=[a]$1")));

   create_string_vec(p, "apple", "banana", "cat", "LERP");
   aprintf("%d, %s, %d\n", 4, print(string_vec, &p), 5);

   create(string, my_str);
   string_append_format(&my_str, "%s", print(string_vec, &p));
   adt_assert(string_equal(&my_str, strw("[ apple, banana, cat, LERP ]")));

   adt_assert(string_length(&my_str) > 2,
      "String is much shorter than expected");

   string_append_format(&my_str, "%s", print(string, strw("DERPYFACE")));

   return EXIT_SUCCESS;
}
