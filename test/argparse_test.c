// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <adt.h>
#include <stdtyp/argparse.h>
#include "test.h"

// Copied from wc
define_args(
   "bytes", 'c', "print the number of bytes", ARG_BOOL, ARG_OPTIONAL,
   "chars", 'm', "print the number of characters", ARG_BOOL, ARG_OPTIONAL,
   "lines", 'l', "print the number of lines", ARG_BOOL, ARG_OPTIONAL,
   "max-line-length", 'L', "print the length of the longest line",
      ARG_BOOL, ARG_OPTIONAL,
   "words", 'w', "print the number of words", ARG_BOOL, ARG_OPTIONAL,
   "version", '\0', "print version information", ARG_BOOL, ARG_OPTIONAL,
   "list", '\0', "list", ARG_BOOL, ARG_OPTIONAL,
   "", '\0', "File", ARG_STRING_ARRAY, ARG_OPTIONAL
);

adt_test(argparse)
{
   char *argv[] = {"argparse_test"};
   create(arg_dict, dict);
   declare_args(&dict);
   parse_args(&dict, argv, 1);
}
