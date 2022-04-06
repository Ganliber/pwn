# 1 "2.3_main.c"
# 1 "<built-in>"
# 1 "<command-line>"
# 31 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 32 "<command-line>" 2
# 1 "2.3_main.c"


extern int shared;
extern void func(int *a, int *b);

int main() {
 int a = 100;
 func(&a, &shared);
 return 0;
}
