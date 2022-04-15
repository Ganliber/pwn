#include "thread.h"
#include <stdio.h>

#define N 100000000

long sum=0;

void do_sum(){
	for(int i=0;i<N;i++){
		sum++;
	}
}

void print(){
       	printf("sum = %ld\n",sum);
}

int main()
{
	create(do_sum);
	printf("do_sum1 begins.\n");
	
	create(do_sum);
	printf("do_sum2 begins. \n");

	join(print);
	printf("do_sum ends.\n");

	return 0;
}
