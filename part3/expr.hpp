#include <stdexcept>
#include <stdlib.h>
#include <stdio.h>
#include <limits>
#include<string>
// Bounds structure - Carries the upper and lower bounds of an expression
template <int L, int U>
struct BOUNDS {
    enum { LOWER=L, UPPER=U };
};

// Helper templates for bounds arithmetic
// Based on https://en.wikipedia.org/wiki/Interval_arithmetic#Interval_operators
template <class b1, class b2>
struct ADD_BOUNDS{
    enum {
        LOWER = b1::LOWER + b2::LOWER,
        UPPER = b1::UPPER + b2::UPPER
    };
    typedef BOUNDS<LOWER, UPPER> RET;
};

template <class b1, class b2>
struct SUB_BOUNDS{
    enum {
        LOWER = b1::LOWER - b2::UPPER,
        UPPER = b1::UPPER - b2::LOWER
    };
    typedef BOUNDS<LOWER, UPPER> RET;
};

template <class b1, class b2>
struct MUL_BOUNDS{
    enum {
        LOWER = std::min(std::min(b1::LOWER * b2::LOWER, b1::LOWER * b2::UPPER), std::min(b1::UPPER * b2::LOWER, b1::UPPER * b2::UPPER)),
        UPPER = std::max(std::max(b1::LOWER * b2::LOWER, b1::LOWER * b2::UPPER), std::max(b1::UPPER * b2::LOWER, b1::UPPER * b2::UPPER)),
    };
    typedef BOUNDS<LOWER, UPPER> RET;
};

template <class b1, class b2>
struct DIV_BOUNDS{
    // There are four different cases to handle for interval division, this first expression figures out
    // which one this is.
    static constexpr int CASE = 
        b2::LOWER > 0 || b2::UPPER < 0 ? 0 :    // Case 0: 0 not in interval
        (b2::UPPER == 0 && b2::LOWER != 0 ? 1 : // Case 1: Upper bound is 0
        (b2::LOWER == 0 && b2::UPPER != 0 ? 2 : // Case 2: Lower bound is 0
        3));                                    // Case 3: lower < 0 < upper or lower = upper = 0

    // The result is calculated using b1 * (1/b2). Here we calculate the upper and lower bounds of 1/b2
    static constexpr float RHS_LOWER = CASE == 0 || CASE == 2 ? 1.0f / b2::UPPER : std::numeric_limits<int>::min();
    static constexpr float RHS_UPPER = CASE == 0 || CASE == 1 ? 1.0f / b2::LOWER : std::numeric_limits<int>::max();
    enum {
        LOWER = (int)std::min(std::min(b1::LOWER * RHS_LOWER, b1::LOWER * RHS_UPPER), std::min(b1::UPPER * RHS_LOWER, b1::UPPER * RHS_UPPER)),
        UPPER = (int)std::max(std::max(b1::LOWER * RHS_LOWER, b1::LOWER * RHS_UPPER), std::max(b1::UPPER * RHS_LOWER, b1::UPPER * RHS_UPPER)),
    };
    typedef BOUNDS<LOWER, UPPER> RET;
};

// Integer literal - eval simply returns the value of the literal
template <int N>
struct LIT
{
    static constexpr int eval(int* vals){
        return N;
    };
    typedef BOUNDS<N, N> bounds;
};

// Variable - Takes a bounds object as a template parameter to determine the range of legal values for this
// variable, which will be factored into the calculation for the bounds of the overall expression.
// During evaluation, is given the value corresponding to the first element in the vals array, which will be
// correct due to the way this array pointer is altered by higher level operations in the overall expression.
template <class B>
struct VAR
{
    static constexpr int eval(int* vals){
        int x = vals[0];
        if (x < B::LOWER || x > B::UPPER){
            throw std::out_of_range("Specified value " + std::to_string(x) + " for the variable is outside of the allowed range");
        }
        return x;
    };
    typedef B bounds;
};

// Helper template which calculates the number of instances of VAR in an expression
// If not a LIT or VAR, simply the sum of VARCOUNT for the left and right hand sides of the expression.
template <class E>
struct VARCOUNT
{
    enum {
        RET = VARCOUNT<typename E::LHS>::RET + VARCOUNT<typename E::RHS>::RET
    };
};

// First base case: VARCOUNT for a VAR is 1
template <class B>
struct VARCOUNT<VAR<B>>
{
    enum {
        RET = 1
    };
};

// Second base case: VARCOUNT for a LIT is 0
template <int N>
struct VARCOUNT<LIT<N>>
{
    enum {
        RET = 0
    };
};

// The four basic arithmetic operations - Addition, Subtraction, Multiplication and Division
// Each template takes two parameters, L and R, which represent the left and right operands. The eval function
// for each will call the eval functions of the operands and apply the relevant operation to their results.

// To make sure that each variable is assigned the correct value from the input array (e.g. the one with index
// matching the variable's position in the expression relative to other variables), the VARCOUNT template above
// is used to determine the number of variables on the left and right hand sides of the expression for ADD SUB MUL
// and DIV. When calling the eval functions of these sub expressions, the whole list can be passed to the left hand
// side, but the right hand side must be given only the section of the list from index L_VARS onwards. The effect
// of this is that when the eval function for a VAR is called, the first value in the list passed to that function
// will be the correct value for the variable.

// Addition
template <class L, class R>
struct ADD
{
    // Expose left and right subexpressions for use by VARCOUNT
    typedef L LHS;
    typedef R RHS;
    enum {
        L_VARS = VARCOUNT<L>::RET,
        R_VARS = VARCOUNT<R>::RET // Unused, included for symmetry
    };
    static constexpr int eval(int* vals){
        int *vals2 = vals;
        for(int i=0;i<L_VARS;i++){
            ++vals2;
        }
        return L::eval(vals) + R::eval(vals2);
    };

    typedef typename ADD_BOUNDS<typename L::bounds, typename R::bounds>::RET bounds;
};

// Subtraction
template <class L, class R>
struct SUB
{
    typedef L LHS;
    typedef R RHS;
    enum {
        L_VARS = VARCOUNT<L>::RET,
        R_VARS = VARCOUNT<R>::RET
    };
    static constexpr int eval(int* vals){
        int *vals2 = vals;
        for(int i=0;i<L_VARS;i++){
            ++vals2;
        }
        return L::eval(vals) - R::eval(vals2);
    };

    typedef typename SUB_BOUNDS<typename L::bounds, typename R::bounds>::RET bounds;
};

// Multiplication
template <class L, class R>
struct MUL
{
    typedef L LHS;
    typedef R RHS;
    enum {
        L_VARS = VARCOUNT<L>::RET,
        R_VARS = VARCOUNT<R>::RET
    };
    static constexpr int eval(int* vals){
        int *vals2 = vals;
        for(int i=0;i<L_VARS;i++){
            ++vals2;
        }
        return L::eval(vals) * R::eval(vals2);
    };

    typedef typename MUL_BOUNDS<typename L::bounds, typename R::bounds>::RET bounds;
};

// Division
// NOTE: Only supports integer division, non-integral results will be truncated.
// An error will be thrown on division by zero.
template <class L, class R>
struct DIV
{
    typedef L LHS;
    typedef R RHS;
    enum {
        L_VARS = VARCOUNT<L>::RET,
        R_VARS = VARCOUNT<R>::RET
    };
    static inline int eval(int* vals){
        int *vals2 = vals;
        for(int i=0;i<L_VARS;i++){
            ++vals2;
        }
        int rhs = R::eval(vals2);
        if (rhs == 0){
            throw std::domain_error("Division by zero is undefined");
        }
        return L::eval(vals) / rhs;
    };

    typedef typename DIV_BOUNDS<typename L::bounds, typename R::bounds>::RET bounds;
};