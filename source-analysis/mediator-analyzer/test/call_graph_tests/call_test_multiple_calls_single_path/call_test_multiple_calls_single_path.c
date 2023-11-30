int main()
{
	int x = 10;
	int y = func_A(x);

	return y;
}

int func_A(int x)
{
	return func_B(x);
}
int func_B(int x)
{
	return func_C(x);
}
int func_C(int x)
{
	return x * x * x;
}

