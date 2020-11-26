.. _min-max:

Specifying Memory Layout
========================

Programs are about more than just numeric values. :ref:`pop-example` describes a program that works on integer values, but most C programs involve changes to values in memory. In addition to describing the return value, specifying most C programs involves describing an initial state of the heap and then relating it to the state of the heap after the program has run. SAW supports specifying programs that involve heaps and pointers.

The specification for ``popcount`` could get away with talking only about the integer values of arguments to a function and its return value. This section introduces ``minmax``, which swaps two pointers if the first pointer's target is greater than the second pointer's target. The return value is ``-1`` if the first pointer's original target was less than the second's, ``0`` if they were equal, and ``1`` if the second pointer's original target was greater than the first's.

A reference implementation of ``minmax`` follows this English specification closely:

.. literalinclude:: examples/intro/minmax.c
  :language: C
  :start-after: // BEGIN MINMAX
  :end-before: // END MINMAX


However, the ordering of the modifications to memory and the comparisons of values can be tricky to get right. Instead of using a C program as the specification, this section will use a specification written in a language called Cryptol.


.. _min-max-cryptol:

Cryptol
-------

:term:`SAWScript` has good facilities for describing memory layouts and pre- and postconditions, but not for specifying algorithms. It is often used together with Cryptol, a domain-specific language for implementing low-level cryptographic algorithms or DSP transforms that reads much like a mathematical description. This helps bridge the gap between formal descriptions and real implementations.

A Cryptol specification for ``minmax`` looks like this:

.. literalinclude:: examples/intro/MinMax.cry
  :language: Cryptol

The first line of the file is a module header. It states that the current module is named ``MinMax``. In this module, there are two definitions: ``minmax``, which specifies the values expected in the pointers' targets after running ``minmax``, and ``minmax_return``, which specifies the value to be returned from ``minmax``.

Each definition begins with a type declaration. These are optional: Cryptol always type checks code, but it can usually infer types on its own. Nevertheless, they make the specification easier to understand. Also, Cryptol's type system is very general, and some of the types that it finds on its own may be complicated. The type of ``minmax`` can be read as "a function that takes an pair of 64-bit values as an argument, and returns a pair of 64-bit values" (the arrow ``->`` separates the argument type from the return type). The type of ``minmax_return`` can be read as "a function that takes a pair of 64-bit values as an argument, and returns a single 8-bit value".

The Cryptol definition of ``minmax`` uses pattern matching to name the first and second elements of the incoming pair as ``x`` and ``y``, respectively. The right side of the ``=`` specifies that the return value is the pair ``(y, x)`` if  ``x`` is greater than ``y``, or the original argument pair ``(x, y)`` otherwise. Because Cryptol's type system doesn't distinguish between signed and unsigned integers, the operator ``>$`` is used for signed comparison, while ``>`` is used for unsigned comparison.

Alternatively, the definition could be written without pattern matching. In this case, the first and second elements of the pair are accessed using the ``.1`` and ``.0`` operators. Pairs can be seen as analogous to structs whose fields are named by numbers.

.. code-block:: Cryptol

    minmax : ([64], [64]) -> ([64], [64])
    minmax pair =
      if pair.0 >$ pair.1
      then (pair.1, pair.0)
      else (pair.0, pair.1)


Cryptol is useful in two different ways in SAW: it is used as a standalone specification language, and it also provides a syntax for explicit expressions in :term:`SAWScript` specification, in which case it occurs in double braces.

The first step in using a Cryptol specification for ``minmax`` is to load the Cryptol module.

.. literalinclude:: examples/intro/minmax.saw
  :language: SAWScript
  :start-after: // BEGIN CRYPTOL_IMPORT
  :end-before: // END CRYPTOL_IMPORT

.. note:: In SAWScript, ``include`` is used to include the contents of a SAWScript file, while ``import`` is used for Cryptol files.



The SAWScript definition ``minmax_ok`` specifies the following:

1. Symbolic integers and pointers to them in the heap are established. ``symbolic_value`` creates a fresh symbolic variable that's accessible from Cryptol, while ``alloc`` creates a pointer to allocated memory of some type (in this case, ``int64_t``). The ``points_to`` operator establishes that the pointer's target is the new symbolic value. This is done twice, once for each argument.

2. The arguments to be provided to ``minmax`` are specified using ``execute``. In this case, the function will be called on the two pointers.

3. The desired targets of the pointers (that is, the values that they should point at after the function call) are specified using ``points_to`` after ``execute``. In this case, the Cryptol ``minmax`` function is called, and the resulting pair is saved in ``result_spec``, which is then used to provide the pointers' targets.

4. The return value is specified in the same manner as that of ``popcount``, by using ``returns``. In this case, rather than specifying the constant ``TRUE``, the result is also given by a Cryptol specification.

.. literalinclude:: examples/intro/minmax.saw
  :language: SAWScript
  :start-after: // BEGIN MINMAX_SPEC
  :end-before: // END MINMAX_SPEC

Note that the Cryptol snippets in double braces can refer to both ``minmax`` and to ``x`` and ``y``. The Cryptol snippets can refer to anything imported from a Cryptol module with ``import``, and also to any name in scope that refers to a :term:`SAWCore` term. In other words, the :term:`SAWScript` name ``x`` can also be used as a Cryptol name to point at a :term:`SAWCore` term.

Finally, verification is invoked just as in ``popcount``, using ``llvm_verify``.

.. literalinclude:: examples/intro/minmax.saw
  :language: SAWScript
  :start-after: // BEGIN MINMAX_VERIFY
  :end-before: // END MINMAX_VERIFY


Exercises: Getting Started with SAW and Pointers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This exercise does not require the use of Cryptol.

1. Write a function that zeroes out the target of a pointer. It should have the following prototype:

   .. code-block:: C

     void zero(uint32_t* x);

2. Write a function ``zero_spec`` that returns ``true`` when ``zero``
   is correct for some input. It should have the following prototype:

   .. code-block:: C

     bool zero_spec(uint32_t x);

3. Use SAW to verify that ``zero_spec`` always returns ``true`` for your implementation of ``zero``.


Exercise: Unsigned Arithmetic
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Create a version of ``minmax`` that specifies its arguments as ``uint64_t`` instead of ``int64_t``, and attempt to verify it using ``minmax_ok``. What does the counterexample tell you about the bug that is introduced?

Exercise: Alternative Implementations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This version of ``minmax`` avoids conditional statements, relying heavily on C's ternary operator. Verify that it fulfills the specification.

.. literalinclude:: examples/intro/minmax.saw
  :language: SAWScript
  :start-after: // BEGIN MINMAX_VERIFY
  :end-before: // END MINMAX_VERIFY


Now, implement a version of ``minmax`` that uses the `XOR swap trick <https://en.wikipedia.org/wiki/XOR_swap_algorithm>`_ to move the values instead of a temporary variable. Verify it.

.. _code-evolution:

Exercise: Swapping and Rotating
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Using SAW, write a specification for a C function that unconditionally swaps the targets of two pointers. Implement the function in C, and verify that it fulfills the specification. Both the specification and the implementation are simpler versions of ``minmax``, and the specification for ``swap`` can be written without a Cryptol specification.

In the course of ordinary software development, requirements change over time. As requirements change, both programs and their specifications must evolve. A verification-oriented workflow can help maintain a correspondence between updated specifcations and code.

Modify the specification so that it describes a function ``rotr3``. After invoking ``rotr3`` on pointers ``x``, ``y``, and ``z``, ``x`` points to the previous target of ``y``, ``y`` points to the previous target of ``z``, and ``z`` points to the previous target of ``x``. Note the error message that occurs when using this specification for ``swap``.

Implement ``rotr3``, and verify it using the new specification.


Exercise: Arrays
~~~~~~~~~~~~~~~~

In SAW, a C array type can be referred to using ``llvm_array``, which
takes the number of elements and their type as arguments. For
instance, ``uint32[3]`` can be represented as ``llvm_array 3 (llvm_int
32)``.  Similarly, the setup value that corresponds to an index in an
array can be referred to using ``element``. For instance, if
``arr`` refers to an array allocated using ``alloc``, then
``element arr 0`` is the first element in ``arr``. These can be
used with ``points_to``.

Write a version of ``rotr3`` that expects its argument to be an array
of three integers. Verify it using SAW.

..  LocalWords:  cryptographic
