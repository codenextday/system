
#include <stdio.h>
#include <string.h>
int main(unsigned int argc, const char *argv[])
{
	if (argc > 1)
		printf("%s %s\n", argv[0], argv[1]);
	printf("Hello\n");
	return 0;
}
