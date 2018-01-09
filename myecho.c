#include"myecho.h"

int echoCommand(char * ch)
{
	int length = strlen(ch), i;
	for(i=5; i<length; i++)
	{
		if (*(ch+i) == '\0')
		{
			*(ch+i) = ' ';
		}
		printf("%c", *(ch+i));
	}
	printf("\n");
	return 0;
}
