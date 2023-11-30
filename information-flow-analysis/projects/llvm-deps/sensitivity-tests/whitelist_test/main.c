// Whitelist example
/*
 *  Variables that are white listed or that are "tainted" by the white-listed
 *  variable are not reported in the vulnerable branch analysis.
 *  even if the downstream white-listed  variable is reassigned to an actual
 *  tainted value the branch/value will not be reported as vulnerable or tainted.
*/
int main(void)
{
  int a, b, c, d, e;
  int key = 4;
  a = key;
  b = a;
  c = b;
  d = a;
  e = c;

  if( a == 3 ) // reported
    ;
  if( b == 3 )
    ;
  if( c == 3 ) // unreported / reported iff bottom part uncommented
    ;
  if( d == 3 ) // reported
    ;

  a = key;
/*
  c = d;
  e = c;
  if ( c == 4 ) // reported
    ;
  if ( e == 3 ) // reported
    ;
*/
  return 0;
}
