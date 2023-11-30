int test_function(int first);

int main()
{

	int x = 72;
	int y;
	y = test_function(x);

	int z;
	z = test_function(y);
}

int test_function(int first)
{
	(first++)*first;
	return first;
}
