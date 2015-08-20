// Copyright 2015 Remi Bernotavicius. All right reserved.

#ifndef __TEST_MAIN_C
#define __TEST_MAIN_C

#include "test.h"

int
main(int argc, char **argv)
{
   for (struct test *test = &__start_test; test != &__stop_test; test++) {
      printf("\n******** Running test %s ********\n", test->name);
      test->func();
   }

   return EXIT_SUCCESS;
}

#endif // __TEST_MAIN_C
