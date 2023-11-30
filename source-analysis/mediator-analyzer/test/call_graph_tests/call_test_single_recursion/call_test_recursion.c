int main()
{
	int x = 5;
	return func_A(x);
}
int func_A(int x)
{
	return func_R(x);
}
int func_R(int x)
{
	if (x == 0)
	{
		func_Z(x);
		return 1;
	}
	else
	{
		return func_R(x-1);
	}
}
int func_Z(int x)
{
	return x + 1;
}
