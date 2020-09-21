.. _swap-example:

First Example: Swapping Two Numbers
===================================

The more of human life that occurs online, the more important it is that we can trust our computers. Bugs in software have led to `death and serious injury <https://en.wikipedia.org/wiki/Therac-25>`_, `public embarassment and economic damage <https://en.wikipedia.org/wiki/Heartbleed>`_, and other serious consequences. Repeated experience has shown that careful reading of code is not enough to prevent serious bugs.

Most developers are used to techniques like testing, continuous integration, and thoughtful documentation that can help prevent mistakes from being introduced into a system during its development. These techniques are relatively inexpensive, but they risk missing certain classes of bugs. For the most important systems, like those that protect human life or information security, it can make sense to use full *verification*, in which a program is mathematically proved to be correct.

This is a complete walk-through of a small verification task, from start to finish. The program to be verified is a C function that swaps two numbers in memory.


The Code
--------

The program to be verified is ``swap``, below:

.. literalinclude:: examples/swap/swap.c
  :language: C
  :start-after: // BEGIN SWAP
  :end-before: // END SWAP

.. index:: specification

``swap`` is correct if, after calling it, the new target of the first pointer is the former target of the second pointer, and the new target of the second pointer is the former target of the first pointer. This description is called a *specification*. A :term:`specification` can be written in a number of formats, including English sentences, but also in machine-readable forms. The advantage of machine-readable specifications is that they can be used as part of an automated workflow.



An example machine-readable specification for ``swap`` is ``swap_spec``:

.. literalinclude:: examples/swap/swap.c
  :language: C
  :start-after: // BEGIN SWAP_SPEC
  :end-before: // END SWAP_SPEC

This specification is written in C. It can be used in a number of ways to produce evidence that ``swap`` is correct. Too keep things concrete, here are three broken versions of ``swap``.

The first incorrect ``swap`` doesn't actually swap its arguments.

.. literalinclude:: examples/swap/swap.c
  :language: C
  :start-after: // BEGIN SWAP_BROKEN1
  :end-before: // END SWAP_BROKEN1

The second incorrect ``swap`` works correctly for most numbers, but not when its first argument is 4142351.

.. literalinclude:: examples/swap/swap.c
  :language: C
  :start-after: // BEGIN SWAP_BROKEN2
  :end-before: // END SWAP_BROKEN2

The third incorrect ``swap`` dereferences a null pointer when its first argument is :math:`2^5` times its second.

.. literalinclude:: examples/swap/swap.c
  :language: C
  :start-after: // BEGIN SWAP_BROKEN3
  :end-before: // END SWAP_BROKEN3

Testing Programs
~~~~~~~~~~~~~~~~

In the course of writing a program, it's common to test it against some pre-determined examples. These can be a good starting point for a test suite. A function like ``chosen_value_test`` can be used to automate this process.

.. literalinclude:: examples/swap/swap.c
  :language: C
  :caption: Testing with pre-chosen values
  :start-after: // BEGIN SWAP_CHOSEN_VALUE_TEST
  :end-before: // END SWAP_CHOSEN_VALUE_TEST

There are some downsides to testing only with chosen values, however. First off, these tests are usually compiled by the author of the code, and there is a risk that important values are not tested. This can be ameliorated by a systematic, disciplined approach to choosing test values, but it can never be completely eliminated. This particular test case is likely to catch ``swap_broken1``, but not the other two.


A second approach to testing is to choose many random values at each execution, as in ``random_value_test``. This approach will eventually find mistakes, but it may not do so in a reasonable amount of time for cases like ``swap_broken2``.

.. literalinclude:: examples/swap/swap.c
  :language: C
  :start-after: // BEGIN SWAP_RANDOM_VALUE_TEST
  :end-before: // END SWAP_RANDOM_VALUE_TEST


Finally, it is possible to exhaustively check the values by enumerating and testing all possible combinations. With In the case of two 32-bit integers, this will take longer than the usual human lifespan, so it is not particularly practical for ongoing software development.

Formal verification is a useful supplement to testing. Like exhaustive testing, verification tools provide full coverage of all inputs, but they need not actually run each case. This is accomplished by reasoning about mathematical models of a program, knocking out huge regions of the state space with single steps. There are many tools and techniques for performing full formal verification, each suitable to different classes of problem. SAW is particularly suited to imperative programs that don't contain potentially-unbounded loops. Verification does tend to be significantly more expensive to implement than systematic testing, both because it requires specialized knowledge and because developing mathematical proofs can take much longer that writing test cases. However, for many programs, automated tools like SAW can be used with similar levels of effort to testing, but resulting in much stronger guarantees. At the same time, checking a proof can sometimes be much faster than testing large parts of the input space, leading to quicker feedback during development.

SAW works in two phases: first, it converts its target program to an internal representation that's more amenable to verification. Then, external solvers are used together with occasional manual guidance to construct proofs.

Symbolic Execution
------------------

.. index:: SAWCore

The internal representation is a language called :term:`SAWCore`, which consists of mathematical functions. For instance, ``swap`` might be converted to a function like:

.. math::

    f(x) = (\mathit{second}(x), \mathit{first}(x))

.. index:: symbolic execution
.. index:: symbolic value
.. index:: concrete value

in which many of the details of pointer arithmetic and memory models have been removed. The conversion process is called *symbolic execution* or *symbolic simulation*. It works by first replacing some of the inputs to a program with *symbolic values*, which are akin to mathematical variables. The term *concrete values* is used to describe honest-to-goodness bits and bytes. As the program runs, operations on symbolic values result in descriptions of operations rather than actual values. Just as adding ``1`` to the concrete value ``5`` yields the concrete value ``6``, adding ``1`` to the symbolic value :math:`x` yields the symbolic value :math:`x+1`. Incrementing the values again yields ``7`` and :math:`x+2`, respectively.

Take the following function

.. code-block:: C

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


When branching on a symbolic value, both paths must be explored. During this exploration, the reason for the branch must also be remembered, and the branches are combined after the exploration. Take, for example:

.. code-block:: C

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

SAW is a tool for extracting functions that model programs, and then applying both automatic and manual reasoning to them. Typically, they will be compared against a :term:`specification` of some kind. SAW uses a framework called Crucible to symbolically execute imperative programs, while it uses custom simulators for other languages. Crucible is an extensible framework - it is capable of symbolically executing LLVM IR, JVM bytecode, x86 machine code, Rust's MIR internal representation, and a number of others.

The first step to using SAW on ``swap`` is to construct its representation in LLVM IR. It is important to pass ``clang`` the ``-O1`` flag, because important symbols are stripped at higher optimization levels, while lower optimization levels yield code that is less amenable to symbolic execution. It can be convenient to include this in a ``Makefile``:

.. literalinclude:: examples/swap/Makefile
  :language: make
  :start-after: # Build commands for swap
  :end-before: # End build commands for swap

.. index:: SAWScript

After building the LLVM bitcode file, the next step is to use SAW to verify that the program meets its :term:`specification`. SAW is controlled using a special-purpose configuration language called :term:`SAWScript`. SAWScript contains commands for loading code artifacts, for describing program specifications, for comparing code artifacts to specifications, and for helping SAW in situations when fully automatic proofs are impossible.

The SAWScript to verify ``swap`` is:

.. literalinclude:: examples/swap/swap.saw
  :language: SAWScript
  :linenos:

There are three steps in this verification task:

1. Line 1 loads the LLVM module to be verified.
2. Lines 3--8 describe a specification to compare the program against.
3. Line 10 instructs SAW to compare a specific function from the LLVM module to the specification.

The LLVM module is loaded using the ``llvm_load_module`` command. This command takes a string that contains the filename as an argument, and results in the module itself. In SAWScript, the results of a command are saved using the ``<-`` operator; here, the name ``swapmod`` is made to refer to the module.

The program specification can be divided into three main components: a precondition, a description of the arguments, and a postcondition. The precondition describes assumptions made in a specification, and it consists of all the commands before ``crucible_execute_func``. The argument description is the call to ``crucible_execute_func`` --- it specifies the arguments that the function will be called with. Finally, the postcondition describes what should be true after the function has been called. In general, the postcondition can describe facts about pointers and memory layout, but here, it describes only the return value.

.. note::

    SAW is a general-purpose framework for combining a number of simulation tools, proof tools, and solvers. Crucible is an extensible symbolic execution framework that serves as the basis for SAW's LLVM support.

Here, the precondition consists of two invocations of ``crucible_fresh_var``, which creates symbolic variables. Internally, these symbolic variables are represented in the internal language :term:`SAWCore`. ``crucible_fresh_var`` takes two arguments: a string, which is a user-chosen name that might show up in error messages, and the type for the symbolic variable. After the precondition, the :term:`SAWScript` variables ``x`` and ``y`` are bound to the respective symbolic values :math:`x` and :math: `y`.

.. index:: term

The function is invoked on these symbolic values using the ``crucible_execute_func`` command. C functions like ``swap`` can be provided with arguments that don't necessarily make sense as pure mathematical values, like pointers or arrays. In SAW, mathematical expressions are called *terms*, while this larger collection of values are called *setup values*. The ``crucible_term`` function is used to create a setup value that consists of a :term:`SAWCore` term. In this case, the symbolic integers are :term:`SAWCore` terms, so both arguments are wrapped in ``crucible_term``.

In the postcondition, it makes sense to specify the return value of the function using ``crucible_return``. In this example, the function is expected to return True, which is represented using the syntax ``{{ 1 : [1] }}``. The curly braces allow terms to be written in a language called Cryptol, which plays an important role in writing SAW specifications. The ``1`` before the colon specifies the Boolean true value, and the ``[1]`` after the colon specifies that it's a 1-bit type (namely, ``bool``).

The entire :term:`specification` is wrapped in ``do { ... }``. This operator allows commands to be built from other commands. In SAW, specifications are written as commands to allow a flexible notation for describing pre- and postconditions. The ``let`` at the top level defines the name ``swap_is_ok`` to refer to this command, which is not yet run.

Translated to English, ``swap_is_ok`` says:

    Let :math:`x` and :math:`y` be 32-bit integers. The result of calling ``swap_spec`` on them is ``true``.

After verification, we know that this is the case *no matter which integers* :math:`x` and :math:`y` are.

.. note::

    :term:`SAWScript` distinguishes between defining a name and saving the result of a command. Use ``let`` to define a name, which may refer to a command or a value, and ``<-`` to run a command and save the result under the given name.

Finally, on line 10, the ``crucible_llvm_verify`` command is used to instruct SAW to carry out verification. The important arguments are ``swapmod``, which specifies the LLVM module that contains the code to be verified; ``"swap_spec"``, the function to be symbolically executed; ``swap_is_ok``, the SAW specification to check ``"swap_spec"`` against; and ``abc``, the name of the solver that will check whether the program satisfies the specification. The other two arguments control the use of helpers and details of symbolic execution, and are described later in this tutorial.

.. TODO link

Verification Failures
---------------------

Not all programs fulfill their :term:`specification`. Sometimes, the specification itself is buggy --- it may refer to earlier versions of standards, contain typos, or just not mean what its authors think it means. Other times, programs can contain bugs. Just as testing failures can provide insight into problems and programs, verification failures can enhance programmers' understanding and help find bugs.

Using a similar :term:`SAWScript` file to attempt to verify ``swap_broken1`` (which simply doesn't swap the inputs) yields the following output in less than half a second::

    [23:52:39.817] ----------Counterexample----------
    [23:52:39.817]   x: 2147483648
    [23:52:39.817]   y: 0
    [23:52:39.817] ----------------------------------

Not only did SAW correctly determine that the program is incorrect, it also provided an example input that can be used for debugging.

For ``swap_broken2``, the following counterexample is found, again in less than half a second::

    [00:07:22.536] ----------Counterexample----------
    [00:07:22.536]   x: 4142351
    [00:07:22.536]   y: 4142350
    [00:07:22.536] ----------------------------------

The value for ``x`` here is the constant in the source code that causes the program to return an incorrect result.

Finally, for ``swap_broken3``, the output notes that a memory load failed during a particular control path. The counterexample provided is::

    [00:11:50.086] ----------Counterexample----------
    [00:11:50.086]   x: 2147483648
    [00:11:50.086]   y: 67108864
    [00:11:50.086] ----------------------------------

2147483648, also known as ``0x4000000``, is indeed the five-bit left shift of 67108864, also known as ``0x80000000``.

Reference Implementations
-------------------------

Another way to provide a specification is in the form of a reference implementation that is clear and trusted. This can be compared to a clever implementation that may be faster or use less memory. In the case of ``swap``, a classic trick to avoid an intermediate variable is to use a sequence of exclusive-or operations to swap the values.

.. literalinclude:: examples/swap/swap.c
  :language: C
  :start-after: // BEGIN XOR_SWAP
  :end-before: // END XOR_SWAP

This approach is much more clever than the straightforward version with an intermediate variable. SAW can be used to check that they are equivalent. The specification (in C) states that ``xor_swap`` has the same effect as ``swap``:

.. literalinclude:: examples/swap/swap.c
  :language: C
  :start-after: // BEGIN XOR_SWAP_SPEC
  :end-before: // END XOR_SWAP_SPEC

The corresponding :term:`SAWScript` is almost identical to that used for ``swap_spec`` --- only the name of the C function was replaced. This is because it is stating the same kind of propery: for all possible values of the appropriate LLVM integer type, the function in question returns ``true``.


Specifications in SAWScript
---------------------------

Most SAW specifications are not written in C. Instead, they are typically written in a combination of :term:`SAWScript` and a language called Cryptol. The specification for ``swap`` can be translated directly to SAWScript itself, as follows:

.. literalinclude:: examples/swap/swap_direct.saw
  :language: SAWScript
  :start-after: // BEGIN SWAP_SPEC
  :end-before: // END SWAP_SPEC

This specification begins by declaring the symbolic values ``x`` and ``y``, just as before. The next step is to establish a pointer to each symbolic value, because ``swap`` takes pointers as arguments. Establishing a pointer consists of two steps: the first, using ``crucible_alloc``, creates a setup value that represents an abstract pointer to nothing in particular; and the second, using ``crucible_points_to``, asserts that the pointer actually points at the symbolic value. In the postcondition (after the ``crucible_execute_func`` call),  ``crucible_points_to`` is used to assert that the pointers now point at the other value.

Writing a :term:`specification` in :term:`SAWScript` instead of C has a number of advantages:

1. Program code that's intended to be executed is cleanly separated from build-time verification code.

2. SAWScript allows specifications or reference implementations in a variety of languages to be used, which enables specifications to be written in the language that best suits them.

3. SAWScript specifications enable a much more precise description of things like memory layouts.

4. SAWScript interfaces with code at the level of LLVM bitcode, rather than C, which means that there is a lower risk of being caught by undefined behavior.

Because SAWScript is a programming language, this specification can be refactored to remove some of the duplication. The first step is to extract the repeated type name:

.. literalinclude:: examples/swap/swap_direct_refactored.saw
  :language: SAWScript
  :start-after: // BEGIN TYPE
  :end-before: // END TYPE

Now, invocations of ``llvm_int 32`` can be replaced with ``i32``.

The next step is to extract the repeated pattern of declaring a symbolic value, allocating a pointer, and arranging for the pointer to point at the value.

.. literalinclude:: examples/swap/swap_direct_refactored.saw
  :language: SAWScript
  :start-after: // BEGIN POINTER_TO_FRESH
  :end-before: // END POINTER_TO_FRESH

``pointer_to_fresh`` is defined to take the arguments ``name`` and ``type``. While ``i32`` is defined as a constant, ``pointer_to_fresh`` is a command, because its right-hand side is a sequence of other commands that are combined using ``do``.  This new command returns both the symbolic value and its pointer in a pair.

The final specification is much shorter:

.. literalinclude:: examples/swap/swap_direct_refactored.saw
  :language: SAWScript
  :start-after: // BEGIN SWAP_SPEC
  :end-before: // END SWAP_SPEC


Cryptol
-------

:term:`SAWScript` has good facilities for describing memory layouts and pre- and postconditions, but not for specifying algorithms. It is often used together with Cryptol, a domain-specific language for implementing low-level cryptographic algorithms or DSP transforms that reads much like a mathematical description. This helps bridge the gap between formal descriptions and real implementations.

A Cryptol specification for ``swap`` looks like this:

.. literalinclude:: examples/swap/Swap.cry
  :language: Cryptol

The first line of the file is a module header. It states that the current module is named ``Swap``. The remainder of the file is a type declaration and a specification. The type declaration reads: "For all types ``a``, ``swap`` maps pairs in which both fields have ``a`` into pairs in which both fields have type ``a``. It is comparable to a signature like ``Pair <A, A> swap<A>(Pair<A, A>)`` in a language like Java. The ``{a}`` introduces the type variable, similarly to the ``<A>`` in the Java signature, and the argument type comes before the ``->``.

The Cryptol definition of ``swap`` uses pattern matching to name the first and second elements of the incoming pair as ``x`` and ``y`` respectively. The right side of the ``=`` specifies the return value as the two elements in opposite positions.

Alternatively, the definition could be written without pattern matching. In this case, the first and second elements of the pair are accessed using the ``.1`` and ``.0`` operators. Pairs can be seen as analogous to structs whose fields are named by numbers.

.. code-block:: Cryptol

    swap pair = (pair.1, pair.0)

Cryptol is useful in two different ways in SAW: it is used as a standalone specification language, and it also provides a syntax for explicit expressions in :term:`SAWScript` specification, in which case it occurs in double braces. For instance, in the first specification for ``swap``, the return value ``{{ 1 : [1] }}`` is a Cryptol expression.

.. TODO add internal link to the {{ 1 : [1] }}

The first step in using a Cryptol specification for ``swap`` is to load the Cryptol module.

.. literalinclude:: examples/swap/swap_cryptol.saw
  :language: SAWScript
  :start-after: // BEGIN CRYPTOL_IMPORT
  :end-before: // END CRYPTOL_IMPORT

After that, the specification uses the double-brace notation to include invocations of Cryptol functions. The syntax ``(swap (x, y)).0`` means to take the first element of the result of swapping ``x`` and ``y``.

.. literalinclude:: examples/swap/swap_cryptol.saw
  :language: SAWScript
  :start-after: // BEGIN SWAP_SPEC
  :end-before: // END SWAP_SPEC

Note that the Cryptol snippets can refer to both ``swap`` and to ``x`` and ``y``. The Cryptol snippets can refer to anything imported from a Cryptol module with ``import``, and also to any name in scope that refers to a :term:`SAWCore` term. In other words, the :term:`SAWScript` name ``x`` can also be used as a Cryptol name to point at a :term:`SAWCore` term.


.. _swap-code-evolution:

Evolving Specifications and Code Together
-----------------------------------------

As needs change, programs and their specifications must evolve. A verification-oriented workflow can help maintain a correspondence between updated specifcations and code.

This section describes the process of adapting ``swap`` to changes in a specification. Please follow along yourself to gain experience with understanding the feedback that SAW provides during this process.

Instead of swapping two numbers, the new version is should rotate three numbers. Expressed in Cryptol, the specification is:

.. literalinclude:: examples/rotr3/Rotr3.cry
  :language: Cryptol

The first step in using this updated specification is to perform a find/replace in the SAWScript specification for ``swap``:

.. literalinclude:: examples/rotr3/rotr3_1.saw
  :language: SAWScript

Invoking SAW on this file yields the following message::

    Cryptol error:
    [error] at rotr3_1.saw:18:51--18:57:
      Type mismatch:
        Expected type: (?it`960, ?it`960, ?it`960)
        Inferred type: ([32], [32])

The meaning of this message is that the type ``([32], [32])`` was found in a context that expected a triple, rather than a pair. In messages, types like ``?it`960`` represent some type that was not discovered during type checking. The reason for this message is that the Cryptol snippet on line 18 column 51 is a pair, but ``rotr3`` expects a triple.

To update the SAWScript to use a triple, a new symbolic value will be necessary. The following script adds a symbolic ``z`` parameter:

.. literalinclude:: examples/rotr3/rotr3_2.saw
  :language: SAWScript

Now, rather than rejecting the script due to a type error, SAW rejects the program with a counterexample::

  ----------Counterexample----------
    x: 0
    y: 2147483648
    z: 0
  ----------------------------------

Indeed, ``swap`` does not place 2147483648 into ``z`` as would be expected by a correct program. The next step is to modify the program:

.. literalinclude:: examples/rotr3/rotr3.c
  :language: C
  :start-after: // BEGIN ROTR3
  :end-before: // END ROTR3

and to update the last line of the SAWScript to verify ``rotr3`` instead:

.. code-block:: SAWScript

    crucible_llvm_verify m "rotr3" [] true rotr3_spec abc;

Once again, SAW fails::

     ----------Counterexample----------
       x: 2147483648
       y: 0
       z: 0
     ----------------------------------

The problem is that, when ``*y`` is assigned the target of ``x``, that target has already been replaced with the target of ``z``. This means that the target of ``z`` ends up in both ``x`` and ``y``. A fixed version uses two temporaries:

.. literalinclude:: examples/rotr3/rotr3.c
  :language: C
  :start-after: // BEGIN ROTR3_FIXED
  :end-before: // END ROTR3_FIXED

SAW accepts this version without complaint, and the bug was prevented.

..  LocalWords:  cryptographic
