#include <stdtyp/vector.h>
#include <stdtyp/string.h>

vector_gen_pod_static(char_vec, char);
vector_gen_pod_static(money_vec, double);
struct TOPY {
   int a;
};
adt_func_static(TOPY);
void TOPY_init(struct TOPY *t) { t->a = 0; }
void TOPY_destroy(struct TOPY *t) { }
void TOPY_copy(struct TOPY *d, const struct TOPY *s) { *d = *s; }
bool TOPY_equal(const struct TOPY *a, const struct TOPY *b)
   { return false; }
void TOPY_print(const struct TOPY *d, struct string *s) { }
vector_gen_ptr_static(ptr_vec, TOPY);
vector_gen_static(topy_vec, TOPY);

int main()
{
   create_string_vec(terp, "apple", "book", "caterpiller");

   create(string_vec, words);
   string_vec_append(&words, strw("apple"));
   string_vec_append(&words, strw("book"));
   string_vec_append(&words, strw("caterpiller"));
   adt_assert_equal(string_vec, &words, &terp);
   adt_assert_equal(string, strw("apple"), string_vec_at(&words, 0));
   adt_assert_equal(string, strw("book"), string_vec_at(&words, 1));
   adt_assert_equal(string, strw("caterpiller"), string_vec_at(&words, 2));

   create(char_vec, mook);
   char_vec_append(&mook, 'a');
   char_vec_append(&mook, 'b');
   char_vec_append(&mook, 'c');
   adt_assert(*char_vec_at(&mook, 0) == 'a');
   adt_assert(*char_vec_at(&mook, 1) == 'b');
   adt_assert(*char_vec_at(&mook, 2) == 'c');

   create(money_vec, money);
   money_vec_append(&money, 4.00);
   money_vec_append(&money, 12.54);
   money_vec_append(&money, 553.23);
   money_vec_append(&money, 332.99);
   money_vec_append(&money, 4.25);
   adt_assert(*money_vec_at(&money, 0) == 4.00);
   adt_assert(*money_vec_at(&money, 1) == 12.54);
   adt_assert(*money_vec_at(&money, 2) == 553.23);
   adt_assert(*money_vec_at(&money, 3) == 332.99);
   adt_assert(*money_vec_at(&money, 4) == 4.25);

   int j = 0;
   iter (money_vec, &money, i) {
      adt_assert(i.key == j);
      adt_assert(*money_vec_at(&money, j) == *i.value);
      j++;
   }

   int cnt = 0;
   iter_value (money_vec, &money, val) {
      adt_assert(*val > 0);
      *val = 7;
      cnt++;
   }
   adt_assert(cnt == 5);

   iter_value (money_vec, &money, val)
      adt_assert(*val == 7);

   create(int_vec, ints);
   int_vec_append(&ints, 1);
   int_vec_append(&ints, 2);
   int_vec_append(&ints, 3);
   int_vec_append(&ints, 4);
   int_vec_append(&ints, 5);
   int_vec_append(&ints, 6);
   int_vec_append(&ints, 6);
   int_vec_insert(&ints, 99, 2);
   int_vec_insert(&ints, 98, 5);
   int_vec_insert(&ints, 97, 0);
   int_vec_insert(&ints, 96, 1);
   int_vec_insert(&ints, 95, 4);
   int_vec_insert(&ints, 94, 3);

   adt_assert(*int_vec_at(&ints, 0) == 97);
   adt_assert(*int_vec_at(&ints, 1) == 96);
   adt_assert(*int_vec_at(&ints, 2) == 1);
   adt_assert(*int_vec_at(&ints, 3) == 94);
   adt_assert(*int_vec_at(&ints, 4) == 2);
   adt_assert(*int_vec_at(&ints, 5) == 95);
   adt_assert(*int_vec_at(&ints, 6) == 99);
   adt_assert(*int_vec_at(&ints, 7) == 3);
   adt_assert(*int_vec_at(&ints, 8) == 4);
   adt_assert(*int_vec_at(&ints, 9) == 98);
   adt_assert(*int_vec_at(&ints, 10) == 5);
   adt_assert(*int_vec_at(&ints, 11) == 6);
   adt_assert(*int_vec_at(&ints, 12) == 6);

   adt_assert(int_vec_index_of(&ints, 97) == 0);
   adt_assert(int_vec_index_of(&ints, 96) == 1);
   adt_assert(int_vec_index_of(&ints, 1) == 2);
   adt_assert(int_vec_index_of(&ints, 94) == 3);
   adt_assert(int_vec_index_of(&ints, 2) == 4);
   adt_assert(int_vec_index_of(&ints, 95) == 5);
   adt_assert(int_vec_index_of(&ints, 99) == 6);
   adt_assert(int_vec_index_of(&ints, 3) == 7);
   adt_assert(int_vec_index_of(&ints, 4) == 8);
   adt_assert(int_vec_index_of(&ints, 98) == 9);
   adt_assert(int_vec_index_of(&ints, 5) == 10);
   adt_assert(int_vec_index_of(&ints, 6) == 11);

   adt_assert(int_vec_remove_value(&ints, 6));

   struct TOPY *t = malloc(sizeof(struct TOPY));
   struct TOPY *u = malloc(sizeof(struct TOPY));
   struct TOPY *v = malloc(sizeof(struct TOPY));
   create(ptr_vec, p);
   ptr_vec_append(&p, t);
   ptr_vec_append(&p, t);
   ptr_vec_append(&p, u);
   ptr_vec_append(&p, u);
   ptr_vec_append(&p, v);
   ptr_vec_append(&p, t);
   ptr_vec_append(&p, v);
   ptr_vec_remove(&p, 3);
   ptr_vec_remove(&p, 5);
   ptr_vec_remove(&p, 0);
   ptr_vec_remove(&p, 0);
   free(t);
   free(u);
   free(v);

   create_string_vec(left, "a", "b", "c");
   create_string_vec(right, "1", "2", "3");
   create_string_vec(expect, "a", "b", "c", "1", "2", "3");

   string_vec_extend(&left, &right);
   adt_assert_equal(string_vec, &left, &expect);

   create_int_vec(sort_test, 4, 5, 2, 3, 7, 6, 8, 9, 1);
   int_vec_sort(&sort_test, int_compare);
   create_int_vec(expect_sort_test, 1, 2, 3, 4, 5, 6, 7, 8, 9);
   adt_assert_equal(int_vec, &sort_test, &expect_sort_test);

   // This is suppose to be okay by design.
   struct int_vec iv a_cleanup(int_vec_destroy) = {};
   int_vec_append(&iv, 9);
   int_vec_append(&iv, 4);
   int_vec_append(&iv, 7);
   int_vec_append(&iv, 1);
   int_vec_append(&iv, 3);
   int_vec_append(&iv, 2);
   int_vec_append(&iv, 8);
   int_vec_append(&iv, 6);
   int_vec_append(&iv, 5);
   int_vec_sort(&iv, int_compare);
   adt_assert_equal(int_vec, &iv, &expect_sort_test);

   // resize
   create(int_vec, resize_vec);
   int_vec_resize(&resize_vec, 10);

   adt_assert_size(int_vec, &resize_vec, 10);
   for (int i = 0; i < 10; i++)
      adt_assert(*int_vec_at(&resize_vec, i) == 0);

   int_vec_resize(&resize_vec, 5);

   adt_assert_size(int_vec, &resize_vec, 5);
   for (int i = 0; i < 5; i++)
      adt_assert(*int_vec_at(&resize_vec, i) == 0);

   return EXIT_SUCCESS;
}
