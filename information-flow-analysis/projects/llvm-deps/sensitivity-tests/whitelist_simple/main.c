int main(void)
{
  int key = 4;
  int a;
  int b;
  int c;

  a =  key;
  b = key;

  if (a == 2) // This line should be reported
    ;
  if (b == 2)
    ;
  return 0;
}
