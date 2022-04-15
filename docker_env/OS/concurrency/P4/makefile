counter-O0.out:concur-demo1.c thread.h
	gcc -O0 concur-demo1.c -o counter-O0.out -lpthread

counter-O1.out:concur-demo1.c thread.h
	gcc -O1 concur-demo1.c -o counter-O1.out -lpthread

counter-O2.out:concur-demo1.c thread.h
	gcc -O2 concur-demo1.c -o counter-O2.out -lpthread

clean:
	rm counter-O0.out counter-O1.out counter-O2.out
