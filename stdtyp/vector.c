#include <stdtyp/vector.h>

#include <string.h>

static void
_memcpy(uint8_t *dst, uint8_t *src, size_t len)
{
   void *t = malloc(len);
   memcpy(t, src, len);
   memcpy(dst, t, len);
   free(t);
}

adt_func_body(vector);

#define BUFFER_START_SIZE (500)

void
vector_init(struct vector *v)
{
   v->buff = malloc(BUFFER_START_SIZE);
   v->buff_len = BUFFER_START_SIZE;
   v->size = 0;
}

void
vector_init_size(struct vector *v, uint64_t s, size_t size)
{
   v->buff = malloc(s * size);
   v->buff_len = s;
   v->size = s;
}

void
vector_destroy(struct vector *v)
{
   free(v->buff);
}

uint64_t
vector_size(const struct vector *v)
{
   return v->size;
}

void
_vector_expand(struct vector *v)
{
   v->buff_len *= 2;
   v->buff = realloc(v->buff, v->buff_len);
}

void *
vector_at(const struct vector *v, uint64_t i, size_t s)
{
   assert(i < v->size);
   uint8_t *b = (uint8_t *)v->buff;
   return &b[i * s];
}

void *
vector_insert(struct vector *v, uint64_t i, size_t s)
{
   while ((v->size + 1) * s > v->buff_len)
      _vector_expand(v);

   uint8_t *b = (uint8_t *)v->buff;

   // If we are the last element, no shifting required, just make size bigger
   // and return last element
   if (i >= v->size) {
      v->size++;
      return &b[(v->size - 1) * s];
   }

   void *pos = &b[i * s];
   // Shift down
   _memcpy(&b[(i + 1) * s], pos, (v->size - i) * s);
   v->size++;

   return pos;
}

void *
vector_extend(struct vector *v, int len, size_t s)
{
   int new_size = (v->size + len) * s;
   if (v->buff_len < new_size) {
      v->buff_len = new_size;
      v->buff = realloc(v->buff, v->buff_len);
   }
   void *p = &v->buff[v->size * s];
   v->size += len;
   return p;
}

void
vector_remove(struct vector *v, uint64_t i, size_t s)
{
   assert(i < v->size);
   if (i < v->size - 1) {
      uint8_t *b = (uint8_t *)v->buff;
      void *pos = &b[i * s];
      // Shift up
      _memcpy(pos, &b[(i + 1) * s], (v->size - (i + 1)) * s);
   }

   v->size--;
}

bool
vector_iterate(const struct vector *v, struct aiter *i, uint64_t *i_out,
   void **v_out, size_t size)
{
   i->ipos++;

   int in = i->ipos - 1;

   if (in >= v->size)
      return false;

   *i_out = in;
   *v_out = vector_at(v, in, size);

   return true;
}

vector_gen_pod_body(int_vec, int);

struct int_vec
int_vec_make_var(int val[], int len)
{
   struct int_vec vec;
   vec.vector.buff_len = len * sizeof(int);
   vec.vector.buff = malloc(vec.vector.buff_len);
   memcpy(vec.vector.buff, val, vec.vector.buff_len);
   vec.vector.size = len;

   return vec;
}

