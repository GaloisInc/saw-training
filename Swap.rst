First Example: Swapping Two Numbers
===================================

This is a complete walk-through of a small verification task, from
start to finish. The problem in question is to swap two ints.


Key concepts:

* Software correctness
  - Correctness requires a specification
  - Specifications can be in English or some kind of code
  - Some specs are close to the problem ("It swaps numbers"), while some are more general and can apply to many things ("it doesn't segfault", "it doesn't leak secrets", etc)
* For important systems, it's far from obvious that they are correct.
  - THERAC, etc
* Evidence for correctness
  - Individual tests
  - Systematic tests
  - Exhaustive tests - enumerate all cases
  - Verification is kinda like exhaustive testing when the search space is too large


Examples:

Start with swap in C. Use this to illustrate the ideas of correctness by comparing to ``xor_swap``, which seems far less obviously OK.

Write a spec in C, like::

    bool swap_ok (int a, int b){
      int *x, *y;
      *x = a;
      *y = b;
      swap(x, y);
      return *x == b && *y == a;
    }

How can we use this to construct evidence that ``swap`` is ok?

Make a few kinds of broken swap:
1. No-op
2. Swap numbers other than 4142351
3. Dereference a null pointer if one number is the other left-shifted by 5 places
(these are clearly easy to spot here, but similar issues can be difficult to find in larger, more complex code)

Choose some test values

Randomly choose some values

Exhaustively check values - figure out how long it takes, and point out the folly

Introduce SAW, and the idea of verification, which gives a "for all values" guarantee without actually exploring the whole search space. Discuss symbolic execution, and show (some representation of) the term that SAW creates for swap. Try these with the broken swaps, and interpret the error messages.


Another way to provide evidence is to compare a reference implementation that is clear and trusted to a clever implementation. Use similar techniques to compare xor_swap and swap.

Specs need not be in C itself - show a Cryptol spec for swap, and repeat the exercise. Discuss only the Cryptol features we use here, and motivate it as a language that is close to the math with extra tools to validate specs.


Finally, replace swap with rotr3, which maps (a,b,c) to (b,c,a). Update all the prior verification code to show how specs have to evolve with code, and show how verification failures can be used in a kind of "TDD" style.

