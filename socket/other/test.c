#include <stdio.h>



int main(int argc, char **argv)
{
	struct a{
		char buff[4];
	};

	printf("%d\n", sizeof(struct a));


	return 0;
}
