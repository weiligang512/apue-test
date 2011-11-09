#include <stdio.h>


int main(int argc, char **argv)
{
	char  c;
	c = getchar();
	fprintf(stdout, "%c", c);
#if 0
	while((c = getchar()) != EOF)
	{
		
	}
#endif
	return 0;
}
