#include <stdio.h>
#include <time.h>


char *fun(void)
{
	char *p;
	
	//puts(p);

	return p;
}
char *fun1(void)
{
	char *p = "123456789";
	
	puts(p);

	return p;
}

int main(int argc, char **argv)
{
	char a = 8;
	char const *const p = &a;
	time_t t;
	struct tm tm;

	p++;
	
	(*p)++;

	puts(p);
	printf("weiligang\n");
	return 0;
}
