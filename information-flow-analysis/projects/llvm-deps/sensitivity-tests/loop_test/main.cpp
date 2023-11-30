
using namespace std;


int main(void)
{
  int arr[14];
  int key = 4;

  // Non constant operands or indicies taint all
  // ConsElems. 14 are generated but when i == 3
  // all get tainted
  for(int i = 0; i < 14; i++){
    arr[i] = i*2;
    if(i == 3){
      arr[i] = key;
    }
  }


  if(arr[0] == 1) // Vulnerable Branch reports this branch
    ;

  return 0;
}
