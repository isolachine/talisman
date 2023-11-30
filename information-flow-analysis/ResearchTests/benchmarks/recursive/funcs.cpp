#include <iostream>
int G(int);

using namespace std;

int F(int a) {
	if (a) {
		cout << "F() returns " << a+1 << endl;
		return G(a+1);
	} else 
		return a;
}

int G(int a) {
	if (a) {
		cout << "G() returns " << a/3 << endl;
		return F(a/3);
	} else 
		return a;
}
