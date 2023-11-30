#include <stdio.h>

void mbed(int secret) {
  size_t wbits;
  size_t i;
  size_t nbits;

  i = secret;
  for (; i < nbits; i++) {
    printf("c");
    wbits <<= 1;
  }
  i = 5;
  i = 6;
  i = 7;

  for (; i < nbits; i++) {
    printf("c");
    wbits <<= 1;
  }
}
