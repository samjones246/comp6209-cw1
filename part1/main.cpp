#include <stdio.h>
#include <stdlib.h>
#include "expr.hpp"

int main(int argc, char **argv){

    // Expression definitions
    typedef ADD<VAR, MUL<SUB<VAR, LIT<2>>, SUB<VAR, LIT<3>>>> FORMULA1;
    typedef DIV<VAR, SUB<VAR, LIT<7>>> FORMULA2;

    // Usage
    if (argc != 2){
        printf("usage: %s X\n", argv[0]);
        printf("Parameters:\n");
        printf("  X - The value to assign to the variable X in the equation\n");
        exit(1);
    }

    // Examples
    int x = atoi(argv[1]);
    int val = FORMULA1::eval(x);
    printf("x = %d\n", x);
    printf("x + (x-2) * (x-3) = %d\n", val);
    val = FORMULA2::eval(x);
    printf("x / (x - 7) = %d\n", val);
}