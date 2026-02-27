#include <stdio.h>


const char* test[] = {"PRINT", "INPUT", "GOTO"};


int main(const char** argv)
{
	for (int i = 0; i < 3; i++) {
		printf("%s\n", test[i]);
	}

	return 0;
}






