// Copyright 2015 Remi Bernotavicius. All right reserved.

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
   struct error (*stream_read)
      (struct stream *, struct string *, size_t, size_t *);
   struct error (*stream_write)(struct stream *, const struct string *);
   bool (*stream_has_more)(struct stream *);
};

extern const struct stream_interface *stream_vtable[];

struct error
stream_read(struct stream *, struct string *)
   a_warn_unused_result;

struct error
stream_read_n(struct stream *, struct string *, size_t)
   a_warn_unused_result;

struct error
stream_read_n_or_less(struct stream *, struct string *, size_t)
   a_warn_unused_result;

struct error
stream_write(struct stream *, const struct string *)
   a_warn_unused_result;

bool
stream_has_more(struct stream *);

#endif // __STREAM_H__
