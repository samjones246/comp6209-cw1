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
    static constexpr int CASE = 
        b2::LOWER > 0 || b2::UPPER < 0 ? 0 :    // Case 0: 0 not in interval
        (b2::UPPER == 0 && b2::LOWER != 0 ? 1 : // Case 1: Upper bound is 0
        (b2::LOWER == 0 && b2::UPPER != 0 ? 2 : // Case 2: Lower bound is 0
        3));                                    // Case 3: lower < 0 < upper or lower = upper = 0
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

// Variable (x) - The single variable of the expression, eval returns the given variable value
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

template <class E>
struct VARCOUNT
{
    enum {
        RET = VARCOUNT<typename E::LHS>::RET + VARCOUNT<typename E::RHS>::RET
    };
};

template <class B>
struct VARCOUNT<VAR<B>>
{
    enum {
        RET = 1
    };
};


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

// -- Addition
template <class L, class R>
struct ADD
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
        return L::eval(vals) + R::eval(vals2);
    };

    typedef typename ADD_BOUNDS<typename L::bounds, typename R::bounds>::RET bounds;
};

// -- Subtraction
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

// -- Multiplication
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

// -- Division
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