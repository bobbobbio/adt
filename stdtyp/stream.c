#include <stdtyp/stream.h>

const struct stream_interface stream_vtable[] = {
   fd_stream_interface,
   string_stream_interface,
};


struct error
stream_read(struct stream *s, struct string *str)
{
   return stream_vtable[s->type].stream_read(s, str, 0, NULL);
}

struct error
stream_read_n(struct stream *s, struct string *str, size_t want)
{
   return stream_vtable[s->type].stream_read(s, str, want, NULL);
}

struct error
stream_read_n_or_less(struct stream *s, struct string *str, size_t want)
{
   size_t _got;
   return stream_vtable[s->type].stream_read(s, str, want, &_got);
}

struct error
stream_write(struct stream *s, struct string *str)
{
   return stream_vtable[s->type].stream_write(s, str);
}

bool
stream_has_more(struct stream *s)
{
   return stream_vtable[s->type].stream_has_more(s);
}
