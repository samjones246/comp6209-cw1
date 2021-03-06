#include <stdexcept>
#include <stdlib.h>
#include <limits>

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
        3));     
    // The result is calculated using b1 * (1/b2). Here we calculate the upper and lower bounds of 1/b2                               // Case 3: lower < 0 < upper or lower = upper = 0
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
    static constexpr int eval(int x){
        return N;
    };
    typedef BOUNDS<N, N> bounds;
};

// Variable (x) - The single variable of the expression, eval returns the given variable value.
// Takes a bounds object as a template parameter to determine the range of legal values for this
// variable, which will be factored into the calculation for the bounds of the overall expression.

template <class B>
struct VAR
{
    static constexpr int eval(int x){
        if (x < B::LOWER || x > B::UPPER){
            throw std::out_of_range("Specified value for the variable is outside of the allowed range");
        }
        return x;
    };
    typedef B bounds;
};

// The four basic arithmetic operations - Addition, Subtraction, Multiplication and Division
// Each template takes two parameters, L and R, which represent the left and right operands. The eval function
// for each will call the eval functions of the operands and apply the relevant operation to their results.

// Bounds for each expression are calculated using the interval arithmetic helper templates above

// Addition
template <class L, class R>
struct ADD
{
    static constexpr int eval(int x){
        return L::eval(x) + R::eval(x);
    };

    typedef typename ADD_BOUNDS<typename L::bounds, typename R::bounds>::RET bounds;
};

// Subtraction
template <class L, class R>
struct SUB
{
    static constexpr int eval(int x){
        return L::eval(x) - R::eval(x);
    };

    typedef typename SUB_BOUNDS<typename L::bounds, typename R::bounds>::RET bounds;
};

// Multiplication
template <class L, class R>
struct MUL
{
    static constexpr int eval(int x){
        return L::eval(x) * R::eval(x);
    };

    typedef typename MUL_BOUNDS<typename L::bounds, typename R::bounds>::RET bounds;
};

// Division
// NOTE: Only supports integer division, non-integral results will be truncated.
// An error will be thrown on division by zero.
template <class L, class R>
struct DIV
{
    static inline int eval(int x){
        int rhs = R::eval(x);
        if (rhs == 0){
            throw std::domain_error("Division by zero is undefined");
        }
        return L::eval(x) / rhs;
    };

    typedef typename DIV_BOUNDS<typename L::bounds, typename R::bounds>::RET bounds;
};