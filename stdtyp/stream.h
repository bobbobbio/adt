#ifndef __STREAM_H__
#define __STREAM_H__

#include <adt.h>

enum stream_type {
   FD_STREAM = 0,
   STRING_STREAM = 1,
};

struct a_packed stream {
   enum stream_type type;
};

struct stream_interface {
   struct error (*stream_read)(struct stream *, struct string *, size_t, size_t *);
   struct error (*stream_write)(struct stream *, struct string *);
   bool (*stream_has_more)(struct stream *);
};

#include <stdtyp/fd_stream.h>
#include <stdtyp/string_stream.h>

extern const struct stream_interface stream_vtable[];

struct error
stream_read(struct stream *, struct string *);

struct error
stream_read_n(struct stream *, struct string *, size_t);

struct error
stream_read_n_or_less(struct stream *, struct string *, size_t, size_t *);

struct error
stream_write(struct stream *, struct string *);

bool
stream_has_more(struct stream *);

#endif // __STREAM_H__
