#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PRINT printf(" ")

int function(int key) {
	int i, j;
	srand(time(NULL));
	
	j = 0;
	
	if(key % 10) // Not reported
		j++;

	for(i = i; i < key; i++) // Not Reported
		j++;

	if(key % 10) // Reported
		PRINT;

	for(i = rand(); i < key; i++) // Reported
		PRINT;
	
	PRINT;
	
	return i;
}


int main(void)
{

  return 0;
}
