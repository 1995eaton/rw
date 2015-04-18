#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <unistd.h>
#include <sys/epoll.h>
#include "util.h"

char *strconcat(const char *a, const char *b) {
  size_t al = strlen(a), bl = strlen(b);
  char *r = malloc((al + bl + 1) * sizeof (char));
  memcpy(r, a, al * sizeof (char));
  memcpy(r + al, b, (bl + 1) * sizeof (char));
  return r;
}

void block(int fd, int ev) {
  struct epoll_event ep = { .events = ev };
  if (epoll_wait(fd, &ep, sizeof ep, -1) == -1)
    error(0, errno, "epoll error");
}

int fd_copy(int ifd, int ofd, size_t bs) {
  if (ifd < 0 || ofd < 0)
    return -1;
  char *buf = malloc(bs * sizeof (char)),
       *pos;
  ssize_t brd,  /* bytes read */
          bwr;  /* bytes written */

  while (( brd = read(ifd, buf, bs) )) {
    if (brd == -1)
      return -1;

    pos = buf;
    while (brd > 0) {
      if ( (bwr = write(ofd, buf, brd)) == -1 ) {
        switch (errno) {
        case EAGAIN:
          block(ifd, EPOLLIN);
          break;
        case EINTR:
          continue;
        default:
          error(0, errno, "write error");
          free(buf);
          return -1;
        }
      }
      brd -= bwr;
      pos += bwr;
    }
  }

  free(buf);
  return 0;
}
