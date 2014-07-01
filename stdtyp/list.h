#include <adt.h>
#include <stdtyp/string.h>

struct list_node {
   void *data;
   struct list_node *next;
   struct list_node *prev;
};
adt_func_header(list_node);

struct list {
   int size;
   struct list_node *head;
   struct list_node *tail;
};
adt_func_header(list);

void list_visit(struct list *l, void (*v)(struct list_node *));
void *list_get(const struct list *l, int i);
void list_insert(struct list *l, void *v, int i);
void *list_remove(struct list *l, int i);
int list_size(const struct list *l);
void list_print_v(const struct list *l, struct string *, void(*p)(void *, struct string *));
void list_copy_v(struct list *d, const struct list *s, void(*c)(void *, void *),
   void*(*n)());

#define _list_gen_header(name, type, type_ref, f) \
   struct name { struct list list; }; \
   _adt_func_header(name, f); \
   f void name##_insert(struct name *, type, int i); \
   f void name##_append(struct name *, type); \
   f void name##_prepend(struct name *, type); \
   f void name##_remove(struct name *, int i); \
   f void name##_get(struct name *, type_ref, int); \
   f type_ref name##_at(struct name *, int); \
   f void name##_init(struct name *); \
   f void name##_destroy(struct name *); \
   f void name##_clear(struct name *); \
   f int name##_size(struct name *)

#define _list_gen_body(name, type, type_ref, type_in, typename, f) \
   _adt_func_body(name, f); \
   f void name##_insert(struct name *a, type v, int i) { \
      type_ref w = typename##_new(); typename##_copy(w, type_in (v)); \
      list_insert(&a->list, w, i); } \
   f void name##_append(struct name *a, type v) { \
      type_ref w = typename##_new(); typename##_copy(w, type_in (v)); \
      list_insert(&a->list, w, list_size(&a->list)); } \
   f void name##_prepend(struct name *a, type v) { \
      type_ref w = typename##_new(); typename##_copy(w, type_in (v)); \
      list_insert(&a->list, w, 0); } \
   f void name##_remove(struct name *a, int i) { \
      typename##_free(list_remove(&a->list, i)); } \
   f void name##_get(struct name *a, type_ref r, int i) { \
      typename##_copy(r, list_get(&a->list, i)); } \
   f type_ref name##_at(struct name *a, int i) { \
      return list_get(&a->list, i); } \
   static void name##_##typename##_free(struct list_node *n) { \
      if (n->data != NULL) \
         typename##_free(n->data);} \
   f void name##_init(struct name *n) { \
      list_init(&n->list); } \
   f void name##_destroy(struct name *a) { \
      list_visit(&a->list, name##_##typename##_free); \
      list_destroy(&a->list); } \
   f int name##_size(struct name *a) { \
      return list_size(&a->list); } \
   static void name##_##typename##_print(void *v, struct string *s) { \
      typename##_print((type_ref)v, s); } \
   f void name##_print(const struct name *a, struct string *s) { \
      list_print_v(&a->list, s, name##_##typename##_print); } \
   f void name##_clear(struct name *a) { \
      while (list_size(&a->list) > 0) name##_remove(a, 0); } \
   static void name##_##typename##_copy(void *d, void *s) { \
      typename##_copy(d, s); } \
   static void *name##_##typename##_new(void *d, void *s) { \
      return typename##_new(); } \
   f void name##_copy(struct name *d, const struct name *s) { \
      name##_clear(d); \
      list_copy_v(&d->list, &s->list, name##_##typename##_copy, \
         name##_##typename##_new); } \
   SWALLOWSEMICOLON

#define ref(x) &x

#define __list_gen_body(name, type, f) \
   _list_gen_body(name, struct type *, struct type *, , type, f)
#define __list_gen_header(name, type, f) \
   _list_gen_header(name, struct type *, struct type *, f)

#define __list_gen_pod_body(name, type, f) \
   m_make(name##_type, type); \
   static void name##_type_print(const type *a, struct string *s) \
      { type##_print(a, s); } \
   _list_gen_body(name, type, type *, ref, name##_type, f)
#define __list_gen_pod_header(name, type, f) \
   _list_gen_header(name, type, type *, f)

#define list_gen_header(name, type) \
   __list_gen_header(name, type, )
#define list_gen_body(name, type) \
   __list_gen_body(name, type, )

#define list_gen_pod_header(name, type) \
   __list_gen_pod_header(name, type, )
#define list_gen_pod_body(name, type) \
   __list_gen_pod_body(name, type, )

#define list_gen_static(name, type) \
   __list_gen_header(name, type, static); \
   __list_gen_body(name, type, static)

#define list_gen_pod_static(name, type) \
   __list_gen_pod_header(name, type, static); \
   __list_gen_pod_body(name, type, static)
