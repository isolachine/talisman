
using namespace std;


typedef struct {
  int a;
  int b;
  int c;
} Pt;

typedef struct {
  int a;
  int b;
  int c;
  Pt* p;
} Point;


int main(void)
{
  Point z;
  z.p = new Pt;
  int key = 4; // tainted value
  z.a = key;   // z.a tainted
  z.b = 0;
  z.c = 3;
  z.p->a = 2;
  z.p->b = key;// z.p->b tainted
  z.p->c = 2;

  if(z.a < 4) // Vulnerable Branch reported
    ;
  if(z.c < 0)
    ;
  if(z.b < 0)
    ;
  if(z.p->a < 4)
    ;
  if(z.p->c < 0)
    ;
  if(z.p->b < 0) // Vulnerable Branch reported
    ;

  return 0;
}
