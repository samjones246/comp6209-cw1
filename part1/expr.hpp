#include <stdexcept>
// Integer literal - eval simply returns the value of the literal
template <int N>
struct LIT
{
    static constexpr int eval(int x){
        return N;
    };
};

// Variable (x) - The single variable of the expression, eval returns the given variable value 
struct VAR
{
    static constexpr int eval(int x){
        return x;
    };
};

// The four basic arithmetic operations - Addition, Subtraction, Multiplication and Division
// Each template takes two parameters, L and R, which represent the left and right operands. The eval function
// for each will call the eval functions of the operands and apply the relevant operation to their results.

// -- Addition
template <class L, class R>
struct ADD
{
    static constexpr int eval(int x){
        return L::eval(x) + R::eval(x);
    };
};

// -- Subtraction
template <class L, class R>
struct SUB
{
    static constexpr int eval(int x){
        return L::eval(x) - R::eval(x);
    };
};

// -- Multiplication
template <class L, class R>
struct MUL
{
    static constexpr int eval(int x){
        return L::eval(x) * R::eval(x);
    };
};

// -- Division
// NOTE: Only supports integer division, non-integral results will be truncated. An error will be thrown on
// division by zero.
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
};