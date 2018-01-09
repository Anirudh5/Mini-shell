#include"mypwd.h"
//#include<stdio.h>
//#include<unistd.h>

int getPwd()
{
	char currDir[1000];
	char * success = getcwd(currDir, sizeof(currDir));
	printf("%s\n", currDir);
	return 1;
}
