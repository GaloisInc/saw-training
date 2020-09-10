First Example: Swapping Two Numbers
===================================

The more of human life that occurs online, the more important it is that we can trust our computers. Bugs in software have led to `death and serious injury <https://en.wikipedia.org/wiki/Therac-25>`_, `public embarassment and economic damage <https://en.wikipedia.org/wiki/Heartbleed>`_, and other serious consequences. Repeated experience has shown that careful reading of code is not enough to prevent serious bugs.

Most developers are used to techniques like testing, continuous integration, and thoughtful documentation that can help prevent mistakes from being introduced into a system during its development. These techniques are relatively inexpensive, but they risk missing certain classes of bugs. For the most important systems, like those that protect human life or information security, it can make sense to use full *verification*, in which a program is mathematically proved to be correct.

This is a complete walk-through of a small verification task, from start to finish. The program to be verified is a C function that swaps two numbers in memory.


* Software correctness

  - Correctness requires a specification
  - Specifications can be in English or some kind of code
  - Some specs are close to the problem ("It swaps numbers"), while some are more general and can apply to many things ("it doesn't segfault", "it doesn't leak secrets", etc)
* Evidence for correctness

  - Individual tests
  - Systematic tests
  - Exhaustive tests - enumerate all cases
  - Verification is kinda like exhaustive testing when the search space is too large


The Code
--------

The program to be verified is ``swap``, below:

.. literalinclude:: examples/swap/swap.c
  :language: C
  :start-after: // BEGIN SWAP
  :end-before: // END SWAP

``swap`` is correct if, after calling it, the new target of the first pointer is the former target of the second pointer, and the new target of the second pointer is the former target of the first pointer. This description is called a *specification*. Specifications can be written in a number of formats, including English sentences, but also in machine-readable forms. The advantage of machine-readable specifications is that they can be used as part of an automated workflow.

An example machine-readable specification for ``swap`` is ``swap_spec``:

.. literalinclude:: examples/swap/swap.c
  :language: C
  :start-after: // BEGIN SWAP_SPEC
  :end-before: // END SWAP_SPEC

This specification is written in C. How can we use this to construct evidence that ``swap`` is ok?



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

Symbolic execution is only typically applicable to programs whose termination doesn't depend on symbolic values. If addition were implemented as:

.. code-block:: C

    unsigned int add(unsigned int x, unsigned int y) {
        for (i = 0; i < y; i ++) {
            x++;
        }
        return x;
    }

then the number of loop iterations would depend on the symbolic value :math:`y`, rather than on some pre-determined concrete number. This means that, each time through the ``for`` loop, two new branches must be explored: one in which the present concrete value of ``i`` is less than the symbolic value of :math:`y`, and on in which it is not. The number of branches to be explored is too large for the execution to terminate in a reasonable amount of time. In other words: symbolic execution is most applicable to programs that "obviously" terminate, or programs in which the number of loop iterations do not depend on which specific input is provided.

Most cryptographic primitives fall into the class of programs for which symbolic execution is a good technique. They typically don't have loops in which the number of iterations depends on specific input values, for instance. 

Running SAW
-----------

SAW is a tool for extracting functions that model programs, and then applying both automatic and manual reasoning to them. Typically, they will be compared against a specification of some kind. SAW uses a framework called Crucible to symbolically execute imperative programs, while it uses custom simulators for other languages. Crucible is an extensible framework - it is capable of symbolically executing LLVM IR, JVM bytecode, x86 machine code, Rust's MIR internal representation, and a number of others.

The first step to using SAW on ``swap`` is to construct its representation in LLVM IR. It is important to pass ``clang`` the ``-O1`` flag, because important symbols are stripped at higher optimization levels, while lower optimization levels yield code that is less amenable to symbolic execution. It can be convenient to include this in a ``Makefile``:

.. literalinclude:: examples/swap/Makefile
  :language: make
  :start-after: # Build commands for swap
  :end-before: # End build commands for swap


After building the LLVM bitcode file, the next step is to use SAW to verify that the program meets its specification. SAW is controlled using a special-purpose configuration language called SAWScript. SAWScript contains commands for loading code artifacts, for describing program specifications, for comparing code artifacts to specifications, and for helping SAW in situations when fully automatic proofs are impossible.

The SAWScript to verify ``swap`` is:

.. literalinclude:: examples/swap/swap.saw
  :linenos:

There are three steps in this verification task:

1. Line 1 loads the LLVM module to be verified.
2. Lines 3--8 describe a specification to compare the program against.
3. Line 10 instructs SAW to compare a specific function from the LLVM module to the specification.

The LLVM module is loaded using the ``llvm_load_module`` command. This command takes a string that contains the filename as an argument, and results in the module itself. In SAWScript, the results of a command are saved using the ``<-`` operator; here, the name ``swapmod`` is made to refer to the module.

The program specification can be divided into three main components: a precondition, a description of the arguments, and a postcondition. The precondition describes assumptions made in a specification, and it consists of all the commands before ``crucible_execute_func``. The argument description is the call to ``crucible_execute_func`` --- it specifies the arguments that the function will be called with. Finally, the postcondition describes what should be true after the function has been called. In general, the postcondition can describe facts about pointers and memory layout, but here, it describes only the return value.

.. note::

    SAW is a general-purpose framework for combining a number of simulation tools, proof tools, and solvers. Crucible is an extensible symbolic execution framework that serves as the basis for SAW's LLVM support.

Here, the precondition consists of two invocations of ``crucible_fresh_var``, which creates symbolic variables. This function takes two arguments: a string, which is a user-chosen name that might show up in error messages, and the type for the symbolic variable. After the precondition, the SAWScript variables ``x`` and ``y`` are bound to the respective symbolic values :math:`x` and :math: `y`.

The function is invoked on these symbolic values using the ``crucible_execute_func`` command. C functions like ``swap`` can be provided with arguments that don't necessarily make sense as pure mathematical values, like pointers or arrays. In SAW, mathematical expressions are called *terms*, while this larger collection of values are called *setup values*. The ``crucible_term`` function is used to create a setup value that consists of a term. In this case, the symbolic integers are terms, so both arguments are wrapped in ``crucible_term``.

In the postcondition, it makes sense to specify the return value of the function using ``crucible_return``. In this example, the function is expected to return True, which is represented using the syntax ``{{ 1 : [1] }}``. The curly braces allow terms to be written in a language called Cryptol, which plays an important role in writing SAW specifications. For now, the ``1`` before the colon specifies the Boolean true value, and the ``[1]`` after the colon specifies that it's a 1-bit type (namely, ``bool``).

The entire specification is wrapped in ``do { ... }``. This operator allows commands to be built from other commands. In SAW, specifications are written as commands to allow a flexible notation for describing pre- and postconditions. The ``let`` at the top level defines the name ``swap_is_ok`` to refer to this command, which is not yet run.

Translated to English, ``swap_is_ok`` says:

    Let :math:`x` and :math:`y` be 32-bit integers. The result of calling ``swap_spec`` on them is ``true``.

After verification, we know that this is the case *no matter which integers :math:`x` and :math:`y` are*.

.. note::

    SAWScript distinguishes between defining a name and saving the result of a command. Use ``let`` to define a name, which may refer to a command or a value, and ``<-`` to run a command and save the result under the given name.

Finally, on line 10, the ``crucible_llvm_verify`` command is used to instruct SAW to carry out verification. The important arguments are ``swapmod``, which specifies the LLVM module that contains the code to be verified; ``"swap_spec"``, the function to be symbolically executed; ``swap_is_ok``, the SAW specification to check ``"swap_spec"`` against; and ``abc``, the name of the solver that will check whether the program satisfies the specification. The other two arguments control the use of helpers and details of symbolic execution, and are described later.

.. TODO link

Verification Failures
---------------------

Not all programs fulfill their specifications. Sometimes, the specification itself is buggy --- it may refer to earlier versions of standards, contain typos, or just not mean what its authors think it means. Other times, programs can contain bugs. Just as testing failures can provide insight into problems and programs, verification failures can enhance programmers' understanding and help find bugs.




Another way to provide evidence is to compare a reference implementation that is clear and trusted to a clever implementation. Use similar techniques to compare xor_swap and swap.

Specs need not be in C itself - show a Cryptol spec for swap, and repeat the exercise. Discuss only the Cryptol features we use here, and motivate it as a language that is close to the math with extra tools to validate specs.




Finally, replace swap with rotr3, which maps (a,b,c) to (b,c,a). Update all the prior verification code to show how specs have to evolve with code, and show how verification failures can be used in a kind of "TDD" style.


..  LocalWords:  cryptographic
