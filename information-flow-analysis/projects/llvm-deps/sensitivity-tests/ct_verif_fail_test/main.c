#include <stdio.h>

int dummy;

int loop(int *sec, int *pub) {
  int i, s1, x;
  s1 = sec[1];
  // x = pub[sec[0]];
  // x = pub[sec[1]];
  // if (s1==0) dummy++;
  // if (i==0) dummy++;
  // for (i=0; i<pub[0]; i++)
  // {
  //   dummy++;
  // }
}

int main(void)
{
  int sec[5]; // tainted value
  int pub[5];

  loop(sec, pub);
  return 0;
}
