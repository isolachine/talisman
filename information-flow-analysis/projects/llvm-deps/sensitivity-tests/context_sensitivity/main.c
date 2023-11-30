#include <stdlib.h>

struct data {
  int a;
  int b;
};

__attribute__((always_inline)) int foo(struct data *d) { return d->a; }
int bar(struct data *d) { return d->a; }
__attribute__((always_inline)) int baz(struct data *d) { return d->a; }

int main_function(struct data *key) {
  struct data nokey1;
  struct data nokey2;
  if (foo(key))
    ;
  if (foo(&nokey1))
    ;

  if (bar(key))
    ;
  if (bar(&nokey2))
    ;

  if (baz(key))
    ;
  if (baz(&nokey2))
    ;
  return 0;
}
