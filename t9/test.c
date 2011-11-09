#include <stdio.h>
#include <unistd.h>


int main(int argc, char **argv)
{
	for (;;)
	{
		fprintf(stdout, ".");
		fflush(stdout);

		sleep(1);
	}

	return 0;
}
