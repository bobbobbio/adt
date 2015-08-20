// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <adt.h>
#include <stdtyp/string.h>
#include <stdtyp/regex.h>
#include <stdtyp/string_stream.h>
#include "test.h"

adt_test(append_format)
{
   create_string(str, "FORMAT: ");
   string_append_format(&str, "%d, %d, %s", 1, 2, "THREE");
   adt_assert(string_equal(&str, strw("FORMAT: 1, 2, THREE")));
}

adt_test(regex)
{
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
}

adt_test(vec_format)
{
   create_string_vec(p, "apple", "banana", "cat", "LERP");
   aprintf("%d, %s, %d\n", 4, print(string_vec, &p), 5);

   create(string, my_str);
   string_append_format(&my_str, "%s", print(string_vec, &p));
   adt_assert(string_equal(&my_str, strw("[ apple, banana, cat, LERP ]")));

   adt_assert(string_length(&my_str) > 2,
      "String is much shorter than expected");

   string_append_format(&my_str, "%s", print(string, strw("DERPYFACE")));
}

adt_test(string_stream)
{
   const struct string *orig = strw("THIS IS A LONG STRING HAHAHAHAHAHHA.");
   create_string_stream(ss, (struct string *)orig);

   create(string, tread);
   ecrash(stream_read_n_or_less((struct stream *)&ss, &tread, 4096));

   assert(string_equal(&tread, orig));
}

adt_test(string_split)
{
   create(string_vec, lvec);
   string_split(strw("a b c d e f g"), ' ', &lvec);
   adt_assert(string_vec_size(&lvec) == 7);
}
