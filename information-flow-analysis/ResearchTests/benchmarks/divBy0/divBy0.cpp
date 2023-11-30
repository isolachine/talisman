#include "stdio.h"
#include "stdlib.h"

unsigned int divd (int n1, int n2) {
	return n1/n2;
}

int main(int argc, char* argv[]) {
	int a, b;
	if (argc <= 1)
		a = 10, b = 1;
	else {
		a = atoi(argv[1]);
		int c = a;
		if (argc <= 2)
			b = 0;
		else
			b = atoi(argv[2]);
	}
	printf("%d\n", divd(a, b));
	return 0;
}
