using namespace std;

typedef struct {
  int x;
  int arr[14];
} MixedInt;

int main(void)
{
  MixedInt m;
  int key = -1;    //Tainted value
  char ctx = 'c';  //Tainted value

  m.x = ctx;        // Tainted m.x

  m.arr[0] = key;   // Tainted m.arr index 0
  m.arr[1] = key;   // Tainted m.arr index 1
  m.arr[2] = 3;
  m.arr[3] = 0;

  if (m.x == 'd')    // Vulnerable branch
    ;

  if (m.arr[3] == 1)// Vulnerable branch
    ;
  if (m.arr[1] == 1)// Vulnerable branch
    ;
  if (m.arr[0] == 1)// Vulnerable branch
    ;

  // Collapsed nodes offer no type information.
  // All elements must be tainted until this is
  // improved.

  return 0;
}
