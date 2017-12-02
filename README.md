# Introduction
This is a robust reverse Polish notation calculator for polynomials with stdin parser and unit tests. All of the code is written in C and the unit test library [cmocka](https://cmocka.org/). The `polynomials` project was written for the 1st year course `Individual Programming Project` at the University of Warsaw.

The documentation was written in Doxygen in Polish. Nevertheless, I will write 

# Polynomials definition
Polynomial is a sum of monomials of a form `px^n` where `x` is a variable name, `n` is a nonnegative integer exponent and `p` is a coefficient. The coefficients of monomials which are a part of a polynomial are pairwise different. A polynomial which is not constant is called `normal`. The coefficient of a monomial is a polynomial of a new variable, different from `x` and from all the variables used in that monomial. In other words, if we denote polynomial variables by `x_0, x_1, x_2, ...` and so on, then the coefficient of a monomial of variable `x_j` (provided it's a normal polynomial) is a polynomial of the variable `x_{j+1}`. The polynomials are sparse, meaning the degree of a polynomial could be much larger than the amount of monomials in the polynomial.

# ...

# What I would do better
..* More decomposition of functions into smaller subfunctions
..* Faster implementation of several polynomial operations
..* Documentation in English


# TODO
1. ~~Explain polynomials in an abstract way~~
2. Describe polynomials operations
3. Describe parsing method with examples
4. Describe unit tests
5. Describe calculator operations (with examples)
6. Write what I would do better, after working several months in the industry
