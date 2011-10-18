#include <stdlib.h>
#include <stdio.h>
int main()
{
    char c = 1;
	int i = 0;
	char buf[512];
    for (; i < 1024; i++)
	{
	     memset(buf,c, 512*sizeof(char));
         fwrite(buf, sizeof(char), 512, stdout);
		 c++;
	}
	return 0;
}
