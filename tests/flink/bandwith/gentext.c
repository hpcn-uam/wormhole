#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	if (argc == 1) {
		printf("Error, too few arguments, need to provide array lenth\n");
		return 1;
	}

	int len = atoi(argv[1]);
	char *v = malloc(len + 1);

	for (int i = 0; i < len; i++) {
		v[i] = (rand() % (126 - 32)) + 32;
	}

	v[len] = 10;

	while (1) {
		fwrite(v, len + 1, 1, stdout);
	}

	return 0;
}
