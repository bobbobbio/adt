#include <stdtyp/vector.h>
#include <stdtyp/string.h>

vector_gen_pod_static(char_vec, char);
vector_gen_pod_static(money_vec, double);
vector_gen_pod_static(int_vec, int);
struct TOPY {
   int a;
};
adt_func_static(TOPY);
void TOPY_init(struct TOPY *t) { t->a = 0; }
void TOPY_destroy(struct TOPY *t) { }
void TOPY_copy(struct TOPY *d, const struct TOPY *s) { *d = *s; }
bool TOPY_equal(const struct TOPY *a, const struct TOPY *b)
   { return false; }
void TOPY_print(const struct TOPY *d) { }
vector_gen_ptr_static(ptr_vec, TOPY);
vector_gen_static(topy_vec, TOPY);

int main()
{
   create_string_vec(terp, "apple", "book", "caterpiller");

   create(string_vec, words);
   string_vec_append(&words, strw("apple"));
   string_vec_append(&words, strw("book"));
   string_vec_append(&words, strw("caterpiller"));
   assert(string_vec_equal(&words, &terp));
   assert(string_equal(strw("apple"), string_vec_at(&words, 0)));
   assert(string_equal(strw("book"), string_vec_at(&words, 1)));
   assert(string_equal(strw("caterpiller"), string_vec_at(&words, 2)));

   create(char_vec, mook);
   char_vec_append(&mook, 'a');
   char_vec_append(&mook, 'b');
   char_vec_append(&mook, 'c');
   assert(*char_vec_at(&mook, 0) == 'a');
   assert(*char_vec_at(&mook, 1) == 'b');
   assert(*char_vec_at(&mook, 2) == 'c');

   create(money_vec, money);
   money_vec_append(&money, 4.00);
   money_vec_append(&money, 12.54);
   money_vec_append(&money, 553.23);
   money_vec_append(&money, 332.99);
   money_vec_append(&money, 4.25);
   assert(*money_vec_at(&money, 0) == 4.00);
   assert(*money_vec_at(&money, 1) == 12.54);
   assert(*money_vec_at(&money, 2) == 553.23);
   assert(*money_vec_at(&money, 3) == 332.99);
   assert(*money_vec_at(&money, 4) == 4.25);

   int j = 0;
   iter (money_vec, &money, i) {
      assert(i.key == j);
      assert(*money_vec_at(&money, j) == *i.value);
      j++;
   }

   int cnt = 0;
   iter_value (money_vec, &money, val) {
      assert(*val > 0);
      *val = 7;
      cnt++;
   }
   assert(cnt == 5);

   iter_value (money_vec, &money, val)
      assert(*val == 7);

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

   assert(*int_vec_at(&ints, 0) == 97);
   assert(*int_vec_at(&ints, 1) == 96);
   assert(*int_vec_at(&ints, 2) == 1);
   assert(*int_vec_at(&ints, 3) == 94);
   assert(*int_vec_at(&ints, 4) == 2);
   assert(*int_vec_at(&ints, 5) == 95);
   assert(*int_vec_at(&ints, 6) == 99);
   assert(*int_vec_at(&ints, 7) == 3);
   assert(*int_vec_at(&ints, 8) == 4);
   assert(*int_vec_at(&ints, 9) == 98);
   assert(*int_vec_at(&ints, 10) == 5);
   assert(*int_vec_at(&ints, 11) == 6);
   assert(*int_vec_at(&ints, 12) == 6);

   assert(int_vec_index_of(&ints, 97) == 0);
   assert(int_vec_index_of(&ints, 96) == 1);
   assert(int_vec_index_of(&ints, 1) == 2);
   assert(int_vec_index_of(&ints, 94) == 3);
   assert(int_vec_index_of(&ints, 2) == 4);
   assert(int_vec_index_of(&ints, 95) == 5);
   assert(int_vec_index_of(&ints, 99) == 6);
   assert(int_vec_index_of(&ints, 3) == 7);
   assert(int_vec_index_of(&ints, 4) == 8);
   assert(int_vec_index_of(&ints, 98) == 9);
   assert(int_vec_index_of(&ints, 5) == 10);
   assert(int_vec_index_of(&ints, 6) == 11);

   assert(int_vec_remove_value(&ints, 6));

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

   return EXIT_SUCCESS;
}
