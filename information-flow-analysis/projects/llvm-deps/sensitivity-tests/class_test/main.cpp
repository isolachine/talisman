using namespace std;

class A {
  public:
  int y;
  char z;
};

int main(void)
{
  A a;
  int key = 4;   // Tainted value
  char pk = 'c'; // Tainted value

  a.y = key;    // a.y is now tainted
  a.z = '0';

  if (a.z == '1')
    ;
  if (a.y == 1) // this branch should be reported
    ;
  return 0;
}
