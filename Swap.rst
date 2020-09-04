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

.. literalinclude:: examples/swap/swap.c
  :language: C
  :start-after: // BEGIN SWAP
  :end-before: // END SWAP

Write a spec in C, like:

.. literalinclude:: examples/swap/swap.c
  :language: C
  :start-after: // BEGIN SWAP_SPEC
  :end-before: // END SWAP_SPEC

How can we use this to construct evidence that ``swap`` is ok?

Make a few kinds of broken swap:
1. No-op
2. Swap numbers other than 4142351
3. Dereference a null pointer if one number is the other left-shifted by 5 places
(these are clearly easy to spot here, but similar issues can be difficult to find in larger, more complex code)

.. literalinclude:: examples/swap/swap.c
  :language: C
  :start-after: // BEGIN SWAP_BROKEN1
  :end-before: // END SWAP_BROKEN1

.. literalinclude:: examples/swap/swap.c
  :language: C
  :start-after: // BEGIN SWAP_BROKEN2
  :end-before: // END SWAP_BROKEN2

.. literalinclude:: examples/swap/swap.c
  :language: C
  :start-after: // BEGIN SWAP_BROKEN3
  :end-before: // END SWAP_BROKEN3


Choose some test values

.. literalinclude:: examples/swap/swap.c
  :language: C
  :start-after: // BEGIN SWAP_CHOSEN_VALUE_TEST
  :end-before: // END SWAP_CHOSEN_VALUE_TEST


Randomly choose some values

.. literalinclude:: examples/swap/swap.c
  :language: C
  :start-after: // BEGIN SWAP_RANDOM_VALUE_TEST
  :end-before: // END SWAP_RANDOM_VALUE_TEST


Exhaustively check values - figure out how long it takes, and point out the folly

Introduce SAW, and the idea of verification, which gives a "for all values" guarantee without actually exploring the whole search space. Discuss symbolic execution, and show (some representation of) the term that SAW creates for swap. Try these with the broken swaps, and interpret the error messages.

SAW works in two phases: first, it converts its target program to an internal representation that's more amenable to verification. Then, external solvers are used together with occasional manual guidance to construct proofs.

Symbolic Execution
------------------

The internal representation is a language called SAWCore, which consists of mathematical functions. For instance, ``swap`` might be converted to a function like:

.. math::

    f(x) = (\mathit{second}(x), \mathit{first}(x))

in which many of the details of pointer arithmetic and memory models have been removed. The conversion process is called *symbolic execution* or *symbolic simulation*. It works by first replacing some of the inputs to a program with *symbolic values*, which are akin to mathematical variables. The term *concrete values* is used to describe honest-to-goodness bits and bytes. As the program runs, operations on symbolic values result in descriptions of operations rather than actual values. Just as adding ``1`` to the concrete value ``5`` yields the concrete value ``6``, adding ``1`` to the symbolic value :math:`x` yields the symbolic value :math:`x+1`. Incrementing the values again yields ``7`` and :math:`x+2`, respectively.

Take the following function::

    int add5(int x) {
        for (int i = 0; i < 5; i++) {
            x = x + 1;
        }
        return x;
    }

The first step in symbolic execution is to create an unknown ``int`` called :math:`y`. Then, the program is run:

1. The first step in ``add5`` is to initialize the for loop. Now, ``x`` is :math:`y` and ``i`` is ``0``.

2. The condition of the ``for`` loop is true, so the body is executed. The program variable ``x`` is incremented by 1. The value of ``x`` is now :math:`y+1`. After the loop body, ``i`` is incremented to ``1``.

3. The condition of the ``for`` loop is again true, so the body is executed again. The program variable ``x`` is incremented by 1. The value of ``x`` is now :math:`y+2`, and ``i`` is incremented to ``2``.

4. These steps are repeated until the ``for`` loop's condition fails because ``i`` is ``5``. At this point, ``x`` is the symbolic value :math:`y + 5`.

5. After the loop, the function returns the value :math:`y+5`.

When provided with the symbolic input :math:`y`, ``add5`` return the symbolic output :math:`y + 5`. This means that it is equivalent to the mathematical function:

.. math::

    f(y) = y + 5


When branching on a symbolic value, both paths must be explored. During this exploration, the reason for the branch must also be remembered, and the branches are combined after the exploration. Take, for example::

    int abs(int x) {
        if (x >= 0) {
            return x;
        } else {
            return (-1) * x;
        }
    }

To symbolically execute it, begin again with ``x`` set to a symbolic int :math:`y`:

1. The first step is to evaluate the expression ``x >= 0``. Because ``x`` is :math:`y`, the result is a new symbolic value :math:`y \geq 0`.

2. The next step is a branch on this value. Because there's no way to know whether :math:`y \geq 0` is true or false, both must be explored.

3. Start with the true case. Here, the interpreter remembers that :math:`y \geq 0` is true, and returns :math:`y`.

4. Next, explore the false case. The interpreter remembers that :math:`y \geq 0` is false, and returns :math:`-y`.

5. Both branches now need to be combined, so the return value represents both cases. The combined symbolic value is an expression that checks whether :math:`y \geq 0`, just as the value of adding ``1`` to :math:`y` is :math:`y+1`.

The resulting function is:

.. math::

  f(x) =
    \begin{cases}
      x & \text{if $x \geq 0$}\\
      -x & \text{if $x < 0$}
    \end{cases}

Symbolic execution is only typically applicable to programs whose termination doesn't depend on symbolic values. If addition were implemented as::

    unsigned int add(unsigned int x, unsigned int y) {
        TODO insert for loop here
    }

Running SAW
-----------

.. literalinclude:: examples/swap/Makefile
  :language: C
  :start-after: # Build commands for swap
  :end-before: # End build commands for swap

Another way to provide evidence is to compare a reference implementation that is clear and trusted to a clever implementation. Use similar techniques to compare xor_swap and swap.

Specs need not be in C itself - show a Cryptol spec for swap, and repeat the exercise. Discuss only the Cryptol features we use here, and motivate it as a language that is close to the math with extra tools to validate specs.




Finally, replace swap with rotr3, which maps (a,b,c) to (b,c,a). Update all the prior verification code to show how specs have to evolve with code, and show how verification failures can be used in a kind of "TDD" style.

