typedef struct {
  int a;
  int b;
  int c;
} Context;


int main(void)
{
  int bad = 4;
  Context c;
  Context d;
  c.b = 0;
  c.c = bad;
  d.b = 0;

  if (c.b == 1) // this would be reported if using AbstractLoc TypeMap only
    ;

  if (c.c == 1) // Vulnerable Branch
    ;

  if (d.b == 1) // Not tainted
    ;

  if (d.c == 1) // Vulnerable Branch
    ;
  return 0;
}
