static char *GLOBAL_VAR = "Hello, World!";

int bar(int a) { return a + 1; }
int baz(int a) {
  if (a == 3) {
    bar(5);
    return 0;
  }
  return 1;
}

int foo(int key, int n, int m) {
  if (m > n) {
    baz(key);
  }
  if (GLOBAL_VAR[2] == key) {
    n++;
    goto end;
  }
  if (GLOBAL_VAR[2] == key) {
    m++;
    return 0;
  }
end:
  bar(m);
  return 1;
}
