int foo(int a, int b) { return a + b; }

int bar(int key, int other) {
  foo(key, 2);
  foo(3, other);
  return 0;
}
