using namespace std;


int main(void)
{
  int key = 4;
  int * x;
  x = new int[20]; // only 1 ConsElem created

  x[0] = 0;
  x[1] = 0;
  x[2] = 3;
  x[5] = key;   // Tainted index 5
  x[11] = key;  // Tainted index 11
  x[13] = 5;
  x[19] = 10;

  // Each of these branches should be reported
  // due to every element represented by one ConsElem
  if ( x[1] == 8)
		;
  if ( x[19] == 1)
		;
  if ( x[2] == 7)
		;
  if ( x[5] == 3)
		;
  if ( x[11] == 2)
		;
  if ( x[13] == 1)
		;
  if ( x[0] == 1)
		;
  return 0;
}
