#include <stdtyp/threading.h>

void
hello(int thread_id)
{
   printf("hello %d\n", thread_id);
}
fptr_define(hello, int);

void
hello2()
{
   printf("hello there\n");
}
fptr_define_noargs(hello2);

int
main(int argc, char **argv)
{
   create(thread_pool, tp);
   for (unsigned i = 0; i < 10; i++)
      thread_pool_run(&tp, hello, i);

   thread_pool_join(&tp);

   for (unsigned i = 0; i < 10; i++)
      thread_pool_run_noargs(&tp, hello2);

   struct thread *t = thread_run(hello, 100);

   thread_join(t);
}
