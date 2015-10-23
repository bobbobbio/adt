// Copyright 2015 Remi Bernotavicius. All right reserved.

#ifndef __TEST_MAIN_C
#define __TEST_MAIN_C

#include "test.h"

int
main(int argc, char **argv)
{
   if (argc != 2) {
      panic("Usage: %s <test-name>", argv[0]);
   }
   create_string(arg, argv[1]);

   for (struct test *test = &__start_test; test != &__stop_test; test++) {
      if (string_equal(&arg, strw("--list"))) {
         printf("%s\n", test->name);
      } else if (string_equal(strw(argv[1]), strw(test->name))) {
         printf("******** Running test %s ********\n", test->name);
         test->func();
         exit(EXIT_SUCCESS);
      }
   }

   if (!string_equal(&arg, strw("--list")))
      panic("Failed to find test %s\n", argv[1]);

   return EXIT_SUCCESS;
}

#endif // __TEST_MAIN_C
