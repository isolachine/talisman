#define CONSTANT_100 100

int do_stuff(int cmd, int other) {
  int a = 0;

  switch (cmd) {
  case CONSTANT_100: // CONSTANT_1 is tainted
  case 10:
    if (other) a = 20; else a = 10;
    break;
  case 50:
    a = 30;
    break;
  default:
    a = 10;
    break;
  }

  return a;
}

int main(void) { return do_stuff(50, 10); }
