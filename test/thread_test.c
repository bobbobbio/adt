// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <stdtyp/threading.h>

create_error_body(test_error);

struct error
raise_test_error(void)
{
   eraise(test_error, "This is a test error.");
}

void
hello(int thread_id)
{
}
fptr_define_void(hello, int);

void
hello2(void)
{
   ehandle(error, raise_test_error()) {
      printf("got a test error, nbd\n");
   }
}
fptr_define_void_noargs(hello2);

int
hello3(int val)
{
   return val * 10;
}
fptr_define(hello3, int, int);

// Since the thread calls itself, this has to go first
struct int_vec *
merge_sort(const struct int_vec *vec);
fptr_define(merge_sort, struct int_vec *, struct int_vec *);

struct int_vec *
merge_sort(const struct int_vec *vec)
{
   // Base case
   if (int_vec_size(vec) <= 1) {
      struct int_vec *n = int_vec_new();
      int_vec_copy(n, vec);
      return n;
   }

   // Split array in half
   create(int_vec, left);
   create(int_vec, right);
   for (unsigned i = 0; i < int_vec_size(vec) / 2; i++)
      int_vec_append(&left, *int_vec_at(vec, i));
   for (unsigned i = int_vec_size(vec) / 2; i < int_vec_size(vec); i++)
      int_vec_append(&right, *int_vec_at(vec, i));

   // Spawn 2 threads to sort the two halves
   struct thread *t1 = thread_run(merge_sort, &left);
   struct thread *t2 = thread_run(merge_sort, &right);

   create_null_ptr(int_vec, new_left);
   create_null_ptr(int_vec, new_right);
   thread_join(t1, &new_left);
   thread_join(t2, &new_right);

   // Merge arrays
   struct int_vec *ret = int_vec_new();

   int i = 0;
   int j = 0;
   while (i < int_vec_size(new_left) || j < int_vec_size(new_right)) {
      if (i == int_vec_size(new_left) ||
            (j < int_vec_size(new_right) &&
            *int_vec_at(new_right, j) < *int_vec_at(new_left, i))) {
         int_vec_append(ret, *int_vec_at(new_right, j));
         j++;
      }
      else if (j == int_vec_size(new_right) ||
            (i < int_vec_size(new_left) &&
            *int_vec_at(new_left, i) <= *int_vec_at(new_right, j))) {
         int_vec_append(ret, *int_vec_at(new_left, i));
         i++;
      }
   }
   adt_assert(int_vec_size(ret) ==
      int_vec_size(new_left) + int_vec_size(new_right));

   return ret;
}

int
main(int argc, char **argv)
{
   create(thread_pool, tp);
   for (unsigned i = 0; i < 10; i++)
      thread_pool_run(&tp, hello, i);

   thread_pool_join(&tp);

   for (unsigned i = 0; i < 10; i++)
      thread_pool_run_noargs(&tp, hello2);

   struct thread *t = thread_run(hello3, 42);

   int val;
   thread_join(t, &val);

   adt_assert(val == 420);

   create_int_vec(to_sort, 8, 3, 4, 2, 7, 1, 8, 2, 1, 4, 7, 4, 7, 2);

   create_null_ptr(int_vec, actual_sorted);
   actual_sorted = merge_sort(&to_sort);

   create_int_vec(expected_sorted, 1, 1, 2, 2, 2, 3, 4, 4, 4, 7, 7, 7, 8, 8);

   adt_assert(int_vec_equal(actual_sorted, &expected_sorted));

   create(mutex, mutex);

   with_mutex (&mutex) {
      printf("Hello from the lock");
   }
}
