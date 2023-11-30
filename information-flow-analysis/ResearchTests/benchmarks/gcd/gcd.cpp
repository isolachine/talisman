#include "stdio.h"
#include "stdlib.h"

unsigned int gcd (unsigned int n1, unsigned int n2) {
	printf("gcd(%d, %d)\n", n1, n2);
	if (n2 == 0)
		return n1;
	else
		return gcd(n2, n1 % n2);
//	return (n2 == 0) ? n1 : gcd(n2, n1 % n2);
}

int main(int argc, char* argv[]) {
	int a = atoi(argv[1]), b = atoi(argv[2]);
    printf("%d\n", gcd(a, b));
    return 0;
}
