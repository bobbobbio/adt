#include <adt.h>
#include <stdtyp/regex.h>

int
main(int argc, char **argv)
{
   create_regex(reg, strw("(.+): (.+)"));
   create(string, key);
   create(string, value);
   assert(regex_match(&reg, strw("derp: 47"), &key, &value));

   assert(string_equal(&key, strw("derp")));
   assert(string_equal(&value, strw("47")));

   create_regex(fmt_regex,
      strw("%([-+# 0])?([0-9]+|\\*)?(.[0-9]+)?(hh|ll|[hljztL])?"
      "([diuoxXfFeEgGaAcspn])"));

   create_const_string(fmt_str, "%2.4ld, %+*.99lli, %p, %@");
   create(string_vec_vec, matches);
   regex_match_all(&fmt_regex, &fmt_str, &matches);

   assert(string_vec_vec_size(&matches) == 3);
   create_string_vec(one, "%2.4ld", "", "2", ".4", "l", "d");
   create_string_vec(two, "%+*.99lli", "+", "*", ".99", "ll", "i");
   create_string_vec(three, "%p", "", "", "", "", "p");
   assert(string_vec_equal(string_vec_vec_at(&matches, 0), &one));
   assert(string_vec_equal(string_vec_vec_at(&matches, 1), &two));
   assert(string_vec_equal(string_vec_vec_at(&matches, 2), &three));
}
