/******************************************************
 *
 * Check if arrays elements can be tainted individually
 *
 *
 ******************************************************/

int main(void) {
  int arr[12];

  int key = 99; // Original tainted value

  arr[0] = key; // index 0 shold be tainted now
  arr[1] = key; // index 1 shold be tainted now
  arr[2] = 1;
  arr[3] = 1;
  arr[11] = key; // index 11 shold be tainted now

  if (arr[0] == 1) // Vulnerable branch reports this branch
    ;
  if (arr[1] == 0) // Vulnerable branch reports this branch
    ;
  if (arr[2] == 1)
    ;
  if (arr[3] == 1)
    ;
  if (arr[11] == 1) // Vulnerable branch reports this branch
    ;
  return 0;
}
