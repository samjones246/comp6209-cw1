#include <stdio.h>
#include <stdlib.h>
#include "expr.hpp"

int main(int argc, char** argv){
    // Usage
    if (argc != 2){
        printf("usage: %s X\n", argv[0]);
        printf("Parameters:\n");
        printf("  X - The value to assign to the variable X in the equation\n");
        exit(1);
    }

    const int l = -5;
    const int u = 5;

    // Expression definitions
    typedef VAR<BOUNDS<l, u>> X;
    typedef ADD<X, MUL<SUB<X, LIT<2>>, SUB<X, LIT<3>>>> FORMULA1;
    typedef DIV<X, LIT<2>> FORMULA2;
    typedef MUL<ADD<X, LIT<3>>, ADD<X, LIT<5>>> FORMULA3;

    // Test cases
    int x = atoi(argv[1]);
    printf("x <= %d <= %d\n", l, u);
    printf("x = %d\n\n", x);

    int lower = FORMULA1::bounds::LOWER;
    int upper = FORMULA1::bounds::UPPER;
    int val = FORMULA1::eval(x);
    printf("f1(x) = x + (x-2) * (x-3)\n");
    printf("%d <= f1(x) <= %d\n", lower, upper);
    printf("f1(x) = %d\n\n", val);

    lower = FORMULA2::bounds::LOWER;
    upper = FORMULA2::bounds::UPPER;
    val = FORMULA2::eval(x);
    printf("f2(x) = x / 2\n");
    printf("%d <= f2(x) <= %d\n", lower, upper);
    printf("f2(x) = %d\n\n", val);

    lower = FORMULA3::bounds::LOWER;
    upper = FORMULA3::bounds::UPPER;
    val = FORMULA3::eval(x);
    printf("f3(x) = (x + 3) * (x + 5)\n");
    printf("%d <= f3(x) <= %d\n", lower, upper);
    printf("f3(x) = %d\n", val);
}