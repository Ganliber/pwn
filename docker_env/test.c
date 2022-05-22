#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void calculate(double a, double b, double c, double delta) {
    if(delta==0) {
        double res = -b/(2*a);
        printf("Two identical solutions: x1 = x2 = %lf\n", res);
    }
    else if(delta>0) {
        double del = sqrt(delta);
        double x1 = (del - b)/(2*a);
        double x2 = (-del - b)/(2*a);
        printf("Two different solutions: x1 = %lf, x2 = %lf\n", x1, x2);
    }
    else {
        double del = sqrt(-delta);
        double x = -b/(2*a);
        printf("Two different complex solutions: x1 = %lf+%lfi, x2 = %lf-%lfi\n",x,del,x,del);
    }
}


int main() {
    double a, b, c, delta;
    scanf("%lf %lf %lf",&a,&b,&c);
    
    if(a<0) {
        a=-a;
        b=-b;
        c=-c;
    }
    else if(a==0) {
        if(b==0&&c==0) printf("This is a identity!\n");
        else if(b==0&&c!=0) printf("No solution!\n");
        else {
            double res = -c/b;
            printf("The given equation is a one-dimensional linear equation,\
             and the solution to the equation is %lf\n",res);
        }
        return 0;
    }
    
    /* One-dimensional quadratic equation */
    delta = b*b - 4*a*c;
    calculate(a, b, c, delta);
    return 0;
}