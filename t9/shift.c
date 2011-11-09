#include <stdio.h>


int main(int argc, char **argv)
{
	unsigned int a = 256;
	int i;
	int b = 256;

	printf("%d\n", a >>= 4);
	
	for(i = 0; i < 32; i++)
	{
		printf("%d", (unsigned int)a & (1 << (31 - i)) ? 1 : 0);
		if((i+1) % 4 == 0)
			printf(" ");
	}
	printf("\n");

	return 0;
}
