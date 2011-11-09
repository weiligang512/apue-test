#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>




int main(int argc, char **argv)
{
	mode_t mask;
      // long int strtol(const char *nptr, char **endptr, int base);
	mask = (mode_t)strtol(argv[1], NULL, 8);

      // mode_t umask(mode_t mask);
	fprintf(stdout, "old umask = %04o, new umask = %04o\n", (int )umask(mask), (int)mask);

	return 0;
}
