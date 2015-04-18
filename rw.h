#ifndef RW_H
#define RW_H

#define RW_TMP_FMT "/rw-XXXXXX.tmp"

typedef struct rw_t {
  char *template;
  int fd, is_open;
  int trunc_on_write;
  size_t block_size;
} rw_t;

void rw_free(rw_t *);
rw_t *rw_create(const char *, size_t, bool);
int rw_stream(rw_t *rw);
int rw_write(rw_t *, const char *);
int rw_write_stdout(rw_t *);

#endif
