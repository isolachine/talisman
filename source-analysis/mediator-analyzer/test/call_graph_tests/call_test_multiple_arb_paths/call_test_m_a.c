int main()
{
	int x = rand_A(5);
	int y = rand_B(10);
	rand_K(x);
	
	rand_C(x, y);
	rand_L(x);

	func_Z(10);
}

int rand_K(int x)
{
	return rand_L(x);
}

int rand_L(int x)
{
	return x - 7;
}

int rand_A(int x)
{
	return x - 5;
}

int rand_B(int x)
{
	return x-7;
}

int rand_C(int x, int y)
{
	int c = func_X(x * y);
	return c;
}

int func_X(int x)
{
	int b = func_Z(x);
	return b;
}

int func_Z(int x)
{
	func_SHOULDNOTSEETHIS(x);
	return x;
}

int func_SHOULDNOTSEETHIS(int x)
{
	return x + 7;
}
