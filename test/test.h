// Copyright 2015 Remi Bernotavicius. All right reserved.

#ifndef __TEST_H
#define __TEST_H

#include <adt.h>

typedef void (*test_function_t)();

#ifdef __APPLE__
// On OS X, it requires the section attribute to be in this format
#define a_section(name) __attribute__ ((section("__DATA," name)))
#elif
#define a_section(name) __attribute__ ((section(name)))
#endif

struct test {
   const char *name;
   test_function_t func;
};

#ifdef __APPLE__
// On linux the linker will provide the following symbols automatically when you
// use the section macro, but on OS X you have to make them yourself.
extern struct test __start_test __asm("section$start$__DATA$test");
extern struct test __stop_test __asm("section$end$__DATA$test");
#endif

// We have to provide the used attribute because otherwise the linker might
// decide to just remove the symbol
#define _adt_test(name, unique_name) \
   static void test_##name(void); \
   struct test unique_name a_section("test") a_used = {#name, &test_##name}; \
   static void test_##name(void)

#define adt_test(name) \
   _adt_test(name, unq(n))

#endif // __TEST_H
