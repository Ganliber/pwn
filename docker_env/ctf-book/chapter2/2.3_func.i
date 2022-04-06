# 1 "2.3_func.c"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 32 "<command-line>" 2
# 1 "2.3_func.c"


int shared = 1;
int tmp = 0;

void func(int *a, int *b) {
 tmp = *a;
 *a = *b;
 *b = tmp;
}
