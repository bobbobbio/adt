#include <adt.h>

#ifndef __APPLE__
#include <extyp/ptrace.h>

int
main(int argc, char **argv)
{
   if (argc < 2) {
      fprintf(stderr, "Usage: %s prog args\n", argv[0]);
      exit(1);
   }

   // Take out the args for the child
   create(string_vec, arg_vec);
   for (unsigned i = 1; i < argc; i++) {
      create_const_string(str, argv[i]);
      string_vec_append(&arg_vec, &str);
   }

   // ptrace!
   create(ptracer, ptracer);
   ecrash(ptracer_subprocess(&ptracer, &arg_vec));
   ptracer_trace(&ptracer);

   return EXIT_SUCCESS;
}

#else

int
main(int argc, char **argv)
{
   panic("ptrace on OS X is a complete joke, this doesn't work.");
}

#endif // __APPLE__
