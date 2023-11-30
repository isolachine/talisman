// #include <stdio.h>
// #include <stdlib.h>

struct st {
  int a;
  int b;
};

struct KeyS {
  int k0;
  int k1;
  union {
    struct {
      int *f0;
      int f1;
    } s0;
    struct {
      int *f0;
      int f1;
      int f2;
    } s1;
  } u;
};

union UU {
  struct {
    int *f0;
    int f1;
  } s0;
  struct {
    int *f0;
    int f1;
    int f2;
  } s1;
  int f5;
} u;

union b {
  int a;
  char c;
  long l;
  struct {
    long l;
    int i;
    char c;
  } s;
};

struct Data {
  struct KeyS *f0;
  int *f1;
};

int regular_func(struct KeyS *a1, int a2);
int regular_func(struct KeyS *a1, int a2) {
  a1->k0 = 1;
  a1->u.s0.f1 = a2;
  return a1->u.s0.f1;
}

int foo(int n, int x, union UU *uu, struct KeyS *key) {
  // struct Data *dataStruct = malloc(sizeof(struct Data));
  // dataStruct->f0 = key;
  // dataStruct->f1 = &i;

  int a[10];
  int b[n];
  int *c;

  a[x] = 1;
  b[x] = 1;
  c[x] = 1;
  int *ptr = &x;
  int val = x;

  struct st s;
  s.a = x;

  union b bb;
  bb.a = x;

  char cc = bb.c;
  long ll = bb.l;

  if (cc) {
  }

  if (bb.c) {
  }

  if (bb.s.l) {
  }

  if (bb.s.i) {
  }

  union UU u;
  u.s0.f1 = x;
  if (u.s1.f1) {
  }

  if (u.s1.f2) {
  }

  struct KeyS aa;
  aa.u.s0.f1 = x;
  if (aa.u.s1.f1) {
  }

  if (regular_func(key, x) >= 1) {
  }

  if (key->k0) {
  }

  if (key->u.s0.f0) {
  }

  // if (regular_func(notkey, i)) {
  //   printf("hello\n");
  // }

  // regular_func(key, &x);
  // if (x) {
  //   printf("x\n");
  // }

  // if (key) {
  //   printf("hello\n");
  // }

  return 0;
}

// int main(void) { return 0; }
