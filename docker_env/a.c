#include<stdio.h>
#include<stdlib.h>

int main()
{
	char Buf[1024];
	short a = (short)0xFFFF;
	int b;
	b = a;
	printf("b=%d \r\n",b);
	b = (a & 0xFFFF);
	printf("b=%d \r\n",b);
	return 1;
}

