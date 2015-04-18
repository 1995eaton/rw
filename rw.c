/* rw - redirect stdin to any file
   The MIT License (MIT)

   Copyright (c) 2015 Jake Eaton

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE. */

#include <stdlib.h>
#include <stdbool.h>
#include <error.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "util.h"
#include "rw.h"

void rw_free(rw_t *rw) {
  if (rw->is_open) {
    close(rw->fd);
    unlink(rw->template);
  }
  free(rw->template);
  free(rw);
}

rw_t *rw_create(const char *tmp_dir, size_t block_size, bool append) {
  rw_t *rw = malloc(sizeof (rw_t));
  rw->trunc_on_write = !append;
  rw->block_size = block_size;
  rw->is_open = 0;
  rw->template = strconcat(tmp_dir == NULL ?
      "/tmp" : tmp_dir, RW_TMP_FMT);
  rw->fd = mkstemps(rw->template, 4 /* length of ".tmp" */);
  if (rw->fd == -1) {
    error(0, errno, "error opening '%s' for writing", rw->template);
    rw_free(rw);
    return NULL;
  }
  rw->is_open = 1;
  return rw;
}

int rw_stream(rw_t *rw) {
  if (fd_copy(STDIN_FILENO, rw->fd, rw->block_size) == -1) {
    error(0, errno, "error copying stdin to '%s'", rw->template);
    rw_free(rw);
    return -1;
  }
  return 0;
}

int rw_write(rw_t *rw, const char *file_name) {
  lseek(rw->fd, 0, SEEK_SET);
  int ofd = open(file_name, O_WRONLY | O_CREAT | (
        rw->trunc_on_write ? O_TRUNC : O_APPEND), 0666);
  if (ofd == -1) {
    error(0, errno, "error opening '%s' for redirection", file_name);
    rw_free(rw);
    return -1;
  }
  if (fd_copy(rw->fd, ofd, rw->block_size) == -1) {
    error(0, errno, "error writing to output file");
    rw_free(rw);
    return -1;
  }
  return 0;
}

int rw_write_stdout(rw_t *rw) {
  lseek(rw->fd, 0, SEEK_SET);
  if (fd_copy(rw->fd, STDOUT_FILENO, rw->block_size) == -1) {
    error(0, errno, "error writing to stdout");
    rw_free(rw);
    return -1;
  }
  return 0;
}
