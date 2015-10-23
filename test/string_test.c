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

a_format(printf, 2, 3)
static void
append_format_va_list_helper(struct string *str, const char *s, ...)
{
   va_list args;
   va_start(args, s);

   string_append_format_va_list(str, s, args);

   va_end(args);
}

adt_test(make_var)
{
   create_string(str, "THIS IS A STRING");

   adt_assert_equal(string, &str, strw("THIS IS A STRING"));
}

adt_test(clear)
{
   create_string(str, "THIS IS A STRING");

   string_clear(&str);

   adt_assert_equal(string, &str, strw(""));
}

adt_test(append_char)
{
   create(string, str);

   string_append_char(&str, 'c');

   adt_assert_equal(string, &str, strw("c"));
}

adt_test(append_int)
{
   create(string, str);

   string_append_int(&str, 2);

   adt_assert_equal(string, &str, strw("2"));
}

adt_test(append_cstring)
{
   create(string, str);

   string_append_cstring(&str, "DERP");

   adt_assert_equal(string, &str, strw("DERP"));
}

adt_test(length)
{
   create_string(str, "123456");

   adt_assert(string_length(&str) == 6);
}

adt_test(char_at_index)
{
   create_string(str, "0123456");

   adt_assert(string_char_at_index(&str, 0) == '0');
   adt_assert(string_char_at_index(&str, 5) == '5');
}

adt_test(to_int)
{
   create_string(str, "589");

   adt_assert(string_to_int(&str) == 589);
}

adt_test(to_int_failure)
{
   create_string(str, "derpcakes");

   adt_assert(string_to_int(&str) == 0);
}

adt_test(equal)
{
   create_string(str, "derpcakes");

   adt_assert_equal(string, &str, strw("derpcakes"));
}

adt_test(strip_easy)
{
   create_string(str, "  ab  ");
   string_strip(&str);
   adt_assert_equal(string, &str, strw("ab"));
}

adt_test(strip_empty)
{
   create_string(str, "");
   string_strip(&str);
   adt_assert_equal(string, &str, strw(""));
}

adt_test(strip_all)
{
   create_string(str, "      \t \n      ");
   string_strip(&str);
   adt_assert_equal(string, &str, strw(""));
}

adt_test(strip_ending)
{
   create_string(str, "a line\r\n");
   string_strip(&str);
   adt_assert_equal(string, &str, strw("a line"));
}

adt_test(split)
{
   create_string(str, "a b c d");

   create(string_vec, vec);
   string_split(&str, ' ', &vec);

   create_string_vec(expected_vec, "a", "b", "c", "d");

   adt_assert_equal(string_vec, &vec, &expected_vec);
}

adt_test(tokenize)
{
   create_string(str, "a b\tc   d\ne");

   create(string_vec, vec);
   string_tokenize(&str, &vec);

   create_string_vec(expected_vec, "a", "b", "c", "d", "e");

   adt_assert_equal(string_vec, &vec, &expected_vec);
}

adt_test(starts_with)
{
   create_string(str, "applecake");

   adt_assert(string_starts_with(&str, strw("apple")));
   adt_assert(!string_starts_with(&str, strw("Apple")));
   adt_assert(!string_starts_with(&str, strw("gpple")));
}

adt_test(ends_with)
{
   create_string(str, "applecake");

   adt_assert(string_ends_with(&str, strw("cake")));
   adt_assert(!string_ends_with(&str, strw("Cake")));
   adt_assert(!string_ends_with(&str, strw("lake")));
}

adt_test(contains_char)
{
   create_string(str, "applecake");

   adt_assert(string_contains_char(&str, 'c'));
   adt_assert(!string_contains_char(&str, 'g'));
}

adt_test(contains_substring)
{
   create_string(str, "applecake");

   adt_assert(string_contains_substring(&str, strw("apple")));
   adt_assert(string_contains_substring(&str, strw("cake")));
   adt_assert(!string_contains_substring(&str, strw("lece")));
}

adt_test(remove_substring)
{
   create_string(str, "applecake");
   create_copy(string, str2, &str);

   string_remove_substring(&str, 5, 8);
   string_remove_substring(&str2, 0, 4);
   adt_assert_equal(string, &str, strw("apple"));
   adt_assert_equal(string, &str2, strw("cake"));
}

adt_test(compare)
{
   create_string(str, "banana");
   create_string(str2, "apple");

   adt_assert(string_compare(&str, &str2) > 0);
   adt_assert(string_compare(&str2, &str) < 0);
   adt_assert(string_compare(&str, &str) == 0);
}

adt_test(hash)
{
   create_string(str, "banana");
   create_string(str2, "apple");

   adt_assert(string_hash(&str) != string_hash(&str2));
   adt_assert(string_hash(&str) == string_hash(&str));
}

adt_test(vec_join)
{
   create_string_vec(vec, "an", "apple", "cake");
   create(string, str);

   string_vec_join(&str, &vec, ' ');
   adt_assert_equal(string, &str, strw("an apple cake"));
}

adt_test(append_format_va_list)
{
   create(string, str);

   append_format_va_list_helper(&str, "%s %s %s", "A", "B", "C");

   adt_assert_equal(string, &str, strw("A B C"));
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

adt_test(vec_format)
{
   create_string_vec(str_vec, "apple", "banana", "cat", "LERP");

   create(string, expected);
   string_append_format(&expected, "%s", print(string_vec, &str_vec));
   adt_assert_equal(string, &expected, strw("[ apple, banana, cat, LERP ]"));
}

adt_test(stream)
{
   const struct string *orig = strw("THIS IS A LONG STRING HAHAHAHAHAHHA.");
   create_string_stream_const(ss, orig);

   create(string, tread);
   ecrash(stream_read_n_or_less((struct stream *)&ss, &tread, 4096));

   assert(string_equal(&tread, orig));
}
