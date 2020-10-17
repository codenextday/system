
#include <stdio.h>
#include <string.h>
int parese_args(char *arg);
int main(int argc, const char *argv[])
{
	if (argc > 1)
		printf("%s %s\n", argv[0], argv[1]);
	parese_args(argv[0]);
	printf("Hello\n");
	return 0;
}
