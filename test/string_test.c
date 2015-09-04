// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <adt.h>
#include <stdtyp/string.h>
#include <stdtyp/regex.h>
#include <stdtyp/string_stream.h>
#include "test.h"

adt_test(string_const)
{
   create_const_string(str, "STRING");
   adt_assert_equal(string, &str, strw("STRING"));

   with_expected_assert_condition("s->mallocd")
      string_append_cstring(&str, "DERP");
}

adt_test(append_format)
{
   create_string(str, "FORMAT: ");
   string_append_format(&str, "%d, %d, %s", 1, 2, "THREE");
   adt_assert_equal(string, &str, strw("FORMAT: 1, 2, THREE"));
}

adt_test(append_format_empty)
{
   create_string(str, "FORMAT: ");
   string_append_format(&str, "derp");
   adt_assert_equal(string, &str, strw("FORMAT: derp"));
}

adt_test(string_replace)
{
   create_string(str, "nothing apple cat apple banana");

   create_regex(reg, strw("apple"));
   string_replace(&str, &reg, strw("steve jobs"));

   adt_assert_equal(string, &str,
      strw("nothing steve jobs cat steve jobs banana"));
}

adt_test(string_replace_regex_group)
{
   create_string(str, "nothing steve jobs cat steve jobs banana");
   create_regex(reg2, strw("(a)"));
   string_replace(&str, &reg2, strw("$1=[$1]\\$1"));

   adt_assert(string_equal(&str,
      strw("nothing steve jobs ca=[a]$1t steve jobs "
      "ba=[a]$1na=[a]$1na=[a]$1")));
}

adt_test(string_append_format)
{
   create(string, str);
   string_append_format(&str, "%d ", 87);
   string_append_format(&str, "%s", print(string, strw("foo")));

   adt_assert_equal(string, &str, strw("87 foo"));
}

adt_test(string_vec_format)
{
   create_string_vec(str_vec, "apple", "banana", "cat", "LERP");

   create(string, expected);
   string_append_format(&expected, "%s", print(string_vec, &str_vec));
   adt_assert_equal(string, &expected, strw("[ apple, banana, cat, LERP ]"));
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
