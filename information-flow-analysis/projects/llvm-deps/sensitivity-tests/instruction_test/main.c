#include <bsd/string.h>
#include <stdio.h>
#include <unistd.h>

int callee(int ict, char *ibuf, int *oct, char *ibeef) {
  if (ict < *oct) {
    if ((strlcpy(ibeef, ibuf, (size_t)ict)) >= *oct)
      return -1; // Truncation Check
    *oct = (size_t)ict;
  } else {
    if ((strlcpy(ibeef, ibuf, (size_t)ict)) >= *oct)
      return -1; // Truncation Check
  }
  return 0;
}

int caller(int fd, char *in) {
  int imax = 100, omax = 90;
  char ibuf[imax], obuf[omax];

  if ((strlcpy(ibuf, in, (size_t)imax)) >= imax)
    goto error; // Truncation Check
  if ((callee(imax, ibuf, &omax, obuf)) < 0)
    goto error;
  if ((write(fd, obuf, (size_t)omax)) == omax)
    return 0;
error:
  return -1;
}
