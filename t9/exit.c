#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int test_exit(void)
{
	fprintf(stdout, "exit\n");

	_exit(1);

	return 0;
}

int main(int argc, char **argv)
{
	test_exit();
	
	puts("success");
	
	return 0;
}
