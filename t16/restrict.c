#include <stdio.h>


int fun(char *restrict x, char *y)
{
	char *a = x, *b = y;

	*x = 6;
	*a = 7;

	*y = 8;
	*b = 9;
	
	return 0;
}


int main(int argc, char **argv)
{
	char a, b;

	fun(&a, &b);
	
	fprintf(stdout, "a = %d, a = %d\n", a, b);




	return 0;
}
