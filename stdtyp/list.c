#include <stdio.h>

#include <stdtyp/list.h>

adt_func_body(list_node);
adt_func_body(list);

void
list_node_init(struct list_node *n)
{
   n->data = NULL;
   n->next = NULL;
   n->prev = NULL;
}

void
list_node_destroy(struct list_node *n)
{
   // Do nothing
}

void
list_node_copy(struct list_node *d, const struct list_node *s)
{
   *d = *s;
}

void
list_init(struct list *l)
{
   l->head = NULL;
   l->tail = NULL;
   l->size = 0;
}

void
list_visit(struct list *l, void (*v)(struct list_node *))
{
   struct list_node *n = l->head;
   while(n != NULL) {
      struct list_node *nn = n->next;
      v(n);
      n = nn;
   }
}

void list_destroy(struct list *l)
{
   list_visit(l, list_node_free);
}

static struct list_node *
_list_get(const struct list *l, int i)
{
   if (i < l->size / 2) {
      int f = 0;
      struct list_node *n = l->head;
      while (n != NULL) {
         if (i == f)
            return n;

         n = n->next;
         f++;
      }
   } else {
      int f = l->size - 1;
      struct list_node *n = l->tail;
      while (n != NULL) {
         if (i == f)
            return n;

         n = n->prev;
         f--;
      }
   }

   return NULL;
}

void
list_copy(struct list *d, const struct list *s)
{
   list_destroy(d);
   list_init(d);

   for(int i = list_size(s) - 1; i >= 0; i--)
      list_insert(d, list_get(s, i), 0);
}

void
list_copy_v(struct list *d, const struct list *s, void(*c)(void *, void *),
   void*(*n)())
{
   for(int i = list_size(s) - 1; i >= 0; i--) {
      void *nn = n();
      list_insert(d, nn, 0);
      c(nn, list_get(s, i));
   }
}

void *
list_get(const struct list *l, int i)
{
   struct list_node *n = _list_get(l, i);
   if (n != NULL)
      return n->data;
   else
      return NULL;
}

void
list_insert(struct list *l, void *v, int i)
{
   struct list_node *n = list_node_new();
   n->data = v;

   if (i == 0) {
      n->next = l->head;
      l->head = n;
      if (n->next == NULL)
         l->tail = n;
      l->size++;

      return;
   } else if (i >= l->size) {
      if (l->tail != NULL)
         l->tail->next = n;
      else
         l->head = n;
      n->prev = l->tail;
      l->tail = n;
      l->size++;

      return;
   }

   struct list_node *fn = _list_get(l, i);
   n->next = fn;
   fn->prev->next = n;
   n->prev = fn->prev;
   fn->prev = n;

   l->size++;
}

void *
list_remove(struct list *l, int i)
{
   struct list_node *n = _list_get(l, i);
   if (n == NULL)
      return NULL;

   if (n->prev != NULL)
      n->prev->next = n->next;
   if (n->next != NULL)
      n->next->prev = n->prev;
   if (n == l->head)
      l->head = n->next;
   if (n == l->tail) 
      l->tail = n->prev;

   void *data = n->data;
   list_node_free(n);

   return data;
}

int
list_size(const struct list *l)
{
   return l->size;
}

static void
void_print_visitor(void *v)
{
   printf("%p", v);
}

void
list_print(const struct list *l)
{
   list_print_v(l, void_print_visitor);
}

void
list_print_v(const struct list *l, void(*p)(void *))
{
   struct list_node *n = l->head;
   if (n == NULL) {
      printf("[ ]");
      return;
   }
   printf("[ ");
   while(n->next != NULL) {
      p(n->data);
      n = n->next;
      printf(", ");
   }
   p(n->data);

   printf(" ]");
}
