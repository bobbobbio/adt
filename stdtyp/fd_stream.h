#ifndef __FILE_STREAM_H__
#define __FILE_STREAM_H__

#include <adt.h>
#include <stdtyp/stream.h>

struct a_packed fd_stream {
   enum stream_type type;
   int fd;
   bool done;
};
adt_func_header(fd_stream);

#define create_fd_stream(name, fd) \
   struct fd_stream name a_cleanup(fd_stream_destroy) = fd_stream_make_var(fd)

void
fd_stream_set_fd(struct fd_stream *s, int fd);

struct fd_stream
fd_stream_make_var(int fd);

struct error
fd_stream_read(struct stream *, struct string *, size_t want, size_t *got);

struct error
fd_stream_write(struct stream *, struct string *);

bool
fd_stream_has_more(struct stream *);

#define fd_stream_interface (struct stream_interface){ \
   .stream_read = fd_stream_read, \
   .stream_write = fd_stream_write, \
   .stream_has_more = fd_stream_has_more \
}

#endif // __FILE_STREAM_H__

