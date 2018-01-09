#include"mycd.h"
#ifndef _STRING_H
#include<string.h>
#endif
#ifndef _STDLIB_H
#include<stdlib.h>
#endif

int changeDir(char ch[])
{
	int len = strlen(ch);
	int i, k=0;
	char ch1[1000];
	for(i=0; i<len; i++)
	{
		if(ch[i] == '\\' && ch[i+1] == ' ')
			continue;
		ch1[k++] = ch[i];
	}
	ch1[k] = '\0';
	//printf("%s\n", ch1);
	int success = chdir(ch1);
	return success;
}
