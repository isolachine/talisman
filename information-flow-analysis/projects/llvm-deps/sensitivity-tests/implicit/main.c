int bar(int ddddd);

int foo(int *arg) {
  *arg = 33;
  if (arg)
    return 3;
  bar(*arg);
  return 200;
}

int main_function(int key) {
  int error = 111;
  int a = key ? 15 : 55;
  if (a == 15) {
    if (key == 5) {
      error = foo(&a);
    } else
      return 1;
  }
  return 0;
}
