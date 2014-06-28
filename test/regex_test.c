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
}
