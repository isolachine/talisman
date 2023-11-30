#include <iostream>
#include <cstdlib>
#include "funcs.h"

using namespace std;

int main(int argc, char* argv[]) {
	int a = 10;
	if (argc > 1) {
		a = atoi(argv[1]);
		cout << "Set input to " << a << endl;
	} else {
		cout << "Default input " << a << endl;
	}

	F(a);

	cout << "Back to main()\n";
	return 0;
}
