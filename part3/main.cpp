#include <stdio.h>
#include <stdlib.h>
#include "expr.hpp"
#include "string"

int main(int argc, char** argv){
    // Example input arrays
    int inputs[3][3] = {
        {3,29,50},
        {-2,20,62},
        {0,12,47},
    };

    // Output arrays: expected and actual
    const int expected[3] = {636,530,220};
    int outputs[3];

    // Variable names (for printing)
    const char* varNames[3] = {"x", "y", "z"};

    // Variable bounds
    const int x_l = -5, x_u = 5;
    const int y_l = 10, y_u = 30;
    const int z_l = 45, z_u = 65;
    int bounds[3][2] = {
        {x_l, x_u},
        {y_l, y_u},
        {z_l, z_u}
    };

    // Expression definitions
    typedef VAR<BOUNDS<x_l, x_u>> X;
    typedef VAR<BOUNDS<y_l, y_u>> Y;
    typedef VAR<BOUNDS<z_l, z_u>> Z;
    typedef DIV<ADD<X, MUL<SUB<Y, LIT<2>>, SUB<Z, LIT<3>>>>,LIT<2>> FORMULA;

    // Display pre-test info
    printf("--- INFO ---\n");

    printf("Test Expression:\n");
    printf("- f(x,y,z) = (x + (y - 2) + (x - 3)) / 2\n");

    printf("Variable Bounds:\n");
    for(int i=0;i<3;i++){
        printf("- %d <= %s <= %d\n", bounds[i][0], varNames[i], bounds[i][1]);
    }

    int lower = FORMULA::bounds::LOWER;
    int upper = FORMULA::bounds::UPPER;
    printf("Statically evaluated expression bounds:\n");
    printf("%d <= f(x,y,z) <= %d\n", lower, upper);

    // Display test results
    printf("--- TEST CASES ---\n");
    for(int t=0;t<3;t++){
        printf("Test #%d:\n", t);
        printf("- Input Values: \n");
        for(int i=0;i<3;i++){
            printf(" - %s = %d\n", varNames[i], inputs[t][i]);
        }
        outputs[t] = FORMULA::eval(inputs[t]);
        printf("- Expected Result: ");
        printf("f(%d,%d,%d) = %d\n", inputs[t][0], inputs[t][1], inputs[t][2], expected[t]);
        printf("- Actual Result: ");
        printf("f(%d,%d,%d) = %d\n", inputs[t][0], inputs[t][1], inputs[t][2], outputs[t]);
        printf("- Correct: ");
        printf(outputs[t] == expected[t] ? "YES\n" : "NO\n");
    }
}