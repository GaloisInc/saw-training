.. _swap-example:

First Example: Swapping Two Numbers
===================================

Most developers are used to techniques like testing, continuous integration, and thoughtful documentation that can help prevent mistakes from being introduced into a system during its development and evolution. These techniques can be relatively effective, but they risk missing certain classes of bugs. For the most important systems, like those that protect human life or information security, it can make sense to also use formal *verification*, in which a program is mathematically proved to be correct for *all* inputs.

What testing does is take the *actual binary executable* and run it on a *subset of inputs* to check for expected outputs. After you've done testing, what you're worried about is that your tests are missing some critical case. Compared to testing, what we do in verification is build a *model of the software* and *prove properties* of that model for *all possible inputs*. When you use verification, what you're worried about is that your model is inaccurate. Fortunately these techniques complement each other: testing can help validate that your model is accurate and verification can help build confidence that you aren't missing uncommon inputs that trigger misbehavior. In combination, testing and verification can build confidence in the *trustworthiness* of a program.

.. This is a complete walk-through of a small verification task, from start to finish. The program to be verified is a C function that swaps two numbers in memory.

In this lesson you'll learn how to use ``SAW`` to build models of functions written in ``C``. You'll learn how to specify what those functions are *supposed* to do, and how to write a *SAWScript* that orchestrates the proof that the functions meet their specifications for all possible inputs.


The Code
--------

The first program to be verified is ``swap``, below:

.. literalinclude:: examples/swap/swap.c
  :language: C
  :start-after: // BEGIN SWAP
  :end-before: // END SWAP

.. index:: specification

Our English specification for ``swap`` is that after calling it, the new target of the first pointer is the former target of the second pointer, and the new target of the second pointer is the former target of the first pointer. The implementer of ``swap`` gets to assume that the pointers are not null, and that they point at initialized memory. This description is called a *specification*. A :term:`specification` can be written in a number of formats, including English sentences, but also in machine-readable forms. The advantage of machine-readable specifications is that they can be used as part of an automated workflow.

An example machine-readable specification, written in ``C``, for ``swap`` is ``swap_spec``:

.. literalinclude:: examples/swap/swap.c
  :language: C
  :start-after: // BEGIN SWAP_SPEC
  :end-before: // END SWAP_SPEC

This specification is a ``C`` function that calls ``swap`` on two provided values and checks that the values are indeed swapped. If they are, it returns ``True``, and if not it returns ``False``. This specification can be used in a number of ways to produce evidence that ``swap`` is correct. 

Exercise: A Broken Swap
~~~~~~~~~~~~~~~~~~~~~~~

Write a buggy variant of ``swap`` that seems like a mistake that could be made by accident. Write another buggy variant of ``swap`` that misbehaves in a way that you think might be difficult to detect. Add them both to ``swap.c`` in the ``examples`` directory.

Testing Programs
~~~~~~~~~~~~~~~~

In the course of writing a program, it's common to test it against some pre-determined examples. These can be a good starting point for a test suite. A function like ``chosen_value_test`` can be used to automate this process.

.. literalinclude:: examples/swap/swap.c
  :language: C
  :start-after: // BEGIN SWAP_CHOSEN_VALUE_TEST
  :end-before: // END SWAP_CHOSEN_VALUE_TEST

There are some downsides to testing only with chosen values, however. First off, these tests are usually selected by the initial author of the code, and there is a risk that important values are not tested, especially as code evolves. This risk can be reduced by a systematic, disciplined approach to choosing test values, but it can never be completely eliminated. This particular test case is likely to catch completely buggy versions of ``swap``, but not subtle or tricky ones.


A second approach to testing is to choose many random values at each execution, as in ``random_value_test``. This approach will eventually find subtle or tricky mistakes, but it may not do so in a reasonable amount of time.

.. literalinclude:: examples/swap/swap.c
  :language: C
  :start-after: // BEGIN SWAP_RANDOM_VALUE_TEST
  :end-before: // END SWAP_RANDOM_VALUE_TEST


Finally, one could attempt to exhaustively check the values by enumerating and testing *all possible* combinations. However, even in the simple case of ``swap``, which only takes two 32-bit integers (resulting in :math:`2^{64}` unique cases), this will take longer than the usual human lifespan, so this technique is not practical for ongoing software development.

The way formal verification addresses this is by reasoning about *mathematical models* of a program, which allows it to eliminate huge regions of the state space with single steps. There are many tools and techniques for performing full formal verification, each suitable to different classes of problem. SAW is particularly suited to imperative programs that don't contain potentially-unbounded loops. In general, the cost of verification is that it requires specialized knowledge and developing mathematical proofs can take much longer than writing test cases. However, for many programs, automated tools like SAW can be used with similar levels of effort to testing, but resulting in much stronger guarantees. At the same time, re-checking a proof can sometimes be *much faster* than testing large parts of the input space, leading to quicker feedback during development.

SAW works in two phases: first it builds a model of the program from the output of ``clang``. Then it uses external solvers and manual guidance to construct a proof that the model meets the specification, or to provide a concrete set of inputs that shows where they differ.


Exercise: Detecting Broken ``swap``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Consider testing-based approaches that are likely to catch the above bugs. Implement them if you'd like. Think about tricky bugs that would thwart those approaches. How confident are you that traditional testing could catch *all* buggy versions of ``swap``? Could you write a test that would catch all possible broken versions of swap, including those written by someone with access to your tests? Roughly how long would it take to run?

Running SAW
-----------

SAW is a tool for extracting models from compiled programs and then applying both automatic and manual reasoning to compare them against a :term:`specification` of some kind. SAW builds models of programs by *symbolically executing* them, and is capable of building models from LLVM bitcode, JVM bytecode, x86 machine code, Rust's MIR internal representation, and a number of other formats.

The first step to using SAW on ``swap`` is to use ``clang`` to construct its representation in LLVM bitcode. It is important to pass ``clang`` the ``-O1`` flag, because important symbols are stripped at higher optimization levels, while lower optimization levels yield code that is less amenable to symbolic execution. It can be convenient to include this rule in a ``Makefile``:

.. literalinclude:: examples/swap/Makefile
  :language: make
  :start-after: # Build commands for swap
  :end-before: # End build commands for swap

.. index:: SAWScript

After building the LLVM bitcode file (by typing ``make swap.bc``), the next step is to use SAW to verify that the program meets its :term:`specification`. SAW is controlled using a special-purpose configuration language called :term:`SAWScript`. SAWScript contains commands for loading code artifacts, for describing program specifications, for comparing code artifacts to specifications, and for helping SAW in situations when fully automatic proofs are impossible.

The SAWScript to verify ``swap`` is:

.. literalinclude:: examples/swap/swap.saw
  :language: SAWScript
  :linenos:

There are three parts to this SAWScript:

1. Lines 1--2 load helper functions and the LLVM module to be verified. This step builds the model from your code.
2. Lines 4--9 set up the symbolic inputs to the ``swap_spec`` function, calls the function on those symbolic inputs and asserts that the return value is ``1`` (True).
3. Line 11 instructs SAW to verify that ``swap_is_ok`` is true for *all possible* input values.

The LLVM module is loaded using the ``llvm_load_module`` command. This command takes a string that contains the filename as an argument, and returns the module itself. In SAWScript, the results of a command are saved using the ``<-`` operator; here, the name ``swapmod`` is made to refer to the module.
   
.. index:: precondition

Here, the precondition consists of creating two symbolic variables. Internally, these symbolic variables are represented in the internal language :term:`SAWCore`. ``variable`` takes three arguments: the type for the symbolic variable, the name for the symbolic variable, a string, which can show up in error messages, and whether the variable is ``FRESH`` (uninitialized) or ``STALE`` (a known value). After the precondition, the :term:`SAWScript` variables ``x`` and ``y`` are bound to the respective symbolic values :math:`x` and :math:`y`.

.. index:: term

The function is invoked on these symbolic values using the ``execute`` command. C functions like ``swap`` can be provided with arguments that don't necessarily make sense as pure mathematical values, like pointers or arrays. In SAW, mathematical expressions are called *terms*, while the larger collection of values are called *setup values*. In this case, the ``x`` and ``y`` terms have components that represent their symbolic values, which is why we pass ``x.s`` and ``y.s`` to ``execute``.

.. note::

   TODO (DJM): please check the last sentence of the previous paragraph for accurate use of terms. Also, I switched from the crucible_... functions to the ones provided by saw-demo / llvm-utils.saw - but I don't know why the things it returns include the fields ``.p``, ``.s`` and ``.t`` - could we write a simpler wrapper that unwraps the ``.s`` for us?

.. index:: postcondition

In the postcondition, the expected return value of the function can be specified using ``returns``. In this example, the function is expected to return ``true``, which is represented using the syntax ``{{ 1 : [1] }}``. The curly braces allow terms to be written in a language called Cryptol, which plays an important role in writing SAW specifications. The ``1`` before the colon specifies the Boolean true value, and the ``[1]`` after the colon specifies that it's a 1-bit type (namely, ``bool``).

The entire :term:`specification` is wrapped in ``do { ... }``. This operator allows commands to be built from other commands. In SAW, specifications are written as commands to allow a flexible notation for describing pre- and postconditions. The ``let`` at the top level defines the name ``swap_is_ok`` to refer to this command, which is not yet run.

Translated to English, ``swap_is_ok`` says:

    Let :math:`x` and :math:`y` be 32-bit integers. The result of calling ``swap_spec`` on them is ``true``.

If verification reports success, we know that this is the case *for all possible values of* :math:`x` *and* :math:`y`.

In other words, ``swap_is_ok`` wraps the C specification ``swap_spec``. The C specification takes care of making sure that the pointer arguments to ``swap`` are non-null, and it checks that the pointers have exchanged targets after calling ``swap``. The SAW wrapper establishes the symbolic values, and ensures that the return value is ``true``.

.. note::

    :term:`SAWScript` distinguishes between defining a name and saving the result of a command. Use ``let`` to define a name, which may refer to a command or a value, and ``<-`` to run a command and save the result under the given name.

Finally, on line 11, the ``crucible_llvm_verify`` command is used to instruct SAW to carry out verification. The important arguments are ``swapmod``, which specifies the LLVM module that contains the code to be verified; ``"swap_spec"``, the function to be symbolically executed; ``swap_is_ok``, and the SAW specification to check ``"swap_spec"`` against. The empty list (``[]``) is an optional list of previously proven statements, which is used in larger verification projects as described :ref:`later in this tutorial<compositional-verification>`. This verification script provides the same level of assurance that exhaustive testing would provide, but it completes in a tiny fraction of the time, fast enough to be part of a standard CI workflow.

Exercises: Getting Started with SAW
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Write a function that zeroes out the target of a pointer. It should have the following prototype:

   .. code-block:: C

     void zero(uint32_t* x);

2. Write a function ``zero_spec`` that returns ``true`` when ``zero``
   is correct for some input. It should have the following prototype:

   .. code-block:: C

     bool zero_spec(uint32_t x);

3. Use SAW to verify that ``zero_spec`` always returns ``true`` for your implementation of ``zero``.


Verification Failures
---------------------

Not all programs fulfill their :term:`specification`. Sometimes, the specification itself is buggy --- it may refer to earlier versions of standards, contain typos, or just not mean what its authors think it means. Other times, programs can contain bugs. Just as testing failures can provide insight into problems and programs, verification failures can enhance programmers' understanding and help find bugs.

.. note::

   TODO: refer to exercise 1 results, or provide a tricky-broken swap here


Using a similar :term:`SAWScript` file to attempt to verify ``swap_broken1`` (which simply doesn't swap the inputs) yields the following output in less than half a second::

    ----------Counterexample----------
      x: 2147483648
      y: 0
    ----------------------------------

Not only did SAW correctly determine that the program is incorrect, it also provided an example input that can be used for debugging. The specific counterexample that is chosen may vary between solvers, particular versions of a solver, or even from run to run.

For ``swap_broken2``, the following counterexample is found, again in less than half a second::

    ----------Counterexample----------
      x: 4142351
      y: 4142350
    ----------------------------------

The value for ``x`` here is the constant in the source code that causes the program to return an incorrect result.

Finally, for ``swap_broken3``, the output notes that a memory load failed during a particular control path. The counterexample provided is::

    ----------Counterexample----------
      x: 2147483648
      y: 67108864
    ----------------------------------

2147483648, also known as ``0x4000000``, is indeed the five-bit left shift of 67108864, also known as ``0x80000000``.

Exercises: Incorrect Versions of ``zero``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Write two incorrect versions of ``zero``.

2. Make a version of ``zero_spec`` for each incorrect version.

3. Use SAW to discover a counterexample for each incorrect version.

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

The corresponding :term:`SAWScript` is almost identical to that used for ``swap_spec`` --- only the name of the C function was replaced. This is because it is stating the same kind of property: for all possible values of the appropriate LLVM integer type, the function in question returns ``true``.


Specifications in SAWScript
---------------------------

Most SAW specifications are not written in C. Instead, they are typically written in a combination of :term:`SAWScript` and a language called Cryptol. The specification for ``swap`` can be translated directly to SAWScript itself, as follows:

.. literalinclude:: examples/swap/swap_direct.saw
  :language: SAWScript
  :start-after: // BEGIN SWAP_SPEC
  :end-before: // END SWAP_SPEC

.. note::
   TODO: update this section to use helper - no crucible_ functions, i32, pointer_to_fresh, etc. (eep - is that in helper?)


This specification begins by declaring the symbolic values ``x`` and ``y``, just as before. The next step is to establish a pointer to each symbolic value, because ``swap`` takes pointers as arguments. Establishing a pointer consists of two steps: the first, using ``crucible_alloc``, creates a setup value that represents an abstract pointer to allocated memory without saying anything about what's in said memory; and the second, using ``crucible_points_to``, asserts that the allocated memory actually contains the symbolic value. Because the precondition sets up the arguments using ``crucible_alloc`` and ``crucible_points_to``, ``swap`` will be called with non-null pointers. In the postcondition (after the ``crucible_execute_func`` call),  ``crucible_points_to`` is used to assert that the pointers now point at the other value.

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


Exercises: Direct Specification for ``zero``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Write a SAW specification for ``zero`` that does not involve a C
   helper. Use the ``swap`` specification in this section as an
   example. The zero for ``uint32_t`` can be written ``{{ 0 : [32] }}``.

2. Use this specification to discover bugs in the incorrect versions
   of ``zero``.

.. _swap-cryptol:

Cryptol
-------

:term:`SAWScript` has good facilities for describing memory layouts and pre- and postconditions, but not for specifying algorithms. It is often used together with Cryptol, a domain-specific language for implementing low-level cryptographic algorithms or DSP transforms that reads much like a mathematical description. This helps bridge the gap between formal descriptions and real implementations.

A Cryptol specification for ``swap`` looks like this:

.. literalinclude:: examples/swap/Swap.cry
  :language: Cryptol

The first line of the file is a module header. It states that the current module is named ``Swap``. The remainder of the file is a type declaration and a specification. The type declaration reads: "For all types ``a``, ``swap`` maps pairs in which both fields have type ``a`` into pairs in which both fields have type ``a``. It is comparable to a signature like ``Pair <A, A> swap<A>(Pair<A, A>)`` in a language like Java. The ``{a}`` introduces the type variable, similarly to the ``<A>`` in the Java signature, and the argument type comes before the ``->``.

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

Instead of swapping two numbers, the new version should rotate three numbers. Expressed in Cryptol, the specification is:

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

Exercise: Arrays
~~~~~~~~~~~~~~~~

In SAW, a C array type can be referred to using ``llvm_array``, which
takes the number of elements and their type as arguments. For
instance, ``uint32[3]`` can be represented as ``llvm_array 3 (llvm_int
32)``.  Similarly, the setup value that corresponds to an index in an
array can be referred to using ``crucible_elem``. For instance, if
``arr`` refers to an array allocated using ``crucible_alloc``, then
``crucible_elem arr 0`` is the first element in ``arr``. These can be
used with ``crucible_points_to``.

Write a version of ``rotr3`` that expects its argument to be an array
of three integers. Verify it using SAW.

..  LocalWords:  cryptographic
