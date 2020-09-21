Compositional Verification and Salsa20
======================================

Most software systems are (pardon the pun) composed of more than a handful of
small functions, and verifying that such systems satisfy a specification
can quickly become unwieldy, as a given function's correctness likely depends
on the correctness of many other functions.

:ref:`swap-example` shows what verification/maintenance for a small standalone
function looks like in practice; this section builds on that work to show how
*compositional verification* can be used to combine the verifications of such
functions and dramatically improve the performance of a verification task.

The task at hand is the verification of an implementation of the Salsa20
encryption algorithm. Complete example code can be found in
``examples/salsa20``.


Salsa20 Verification Overview
-----------------------------

Salsa20 is a stream cipher developed in 2005 by Daniel J. Bernstein, built on a
pseudorandom function utilizing add-rotate-XOR (ARX) operations on 32-bit words
[#salsa20wiki]_. His original specification can be found
`here <http://cr.yp.to/snuffle/spec.pdf>`_, for the mathematically inclined.

The specification for this task is a trusted implementation written in
:term:`Cryptol`. This is analogous to what is covered in :ref:`swap-cryptol` in
the ``swap`` example, but for a larger system. Some exemplars from this
specification are explored below for the sake of showing what large-scale
Cryptol programs look like.

The implementation to be verified is written in C to closely match the Cryptol
specification. In practice, this implementation would likely make use of tricks
akin to the XOR-based swap explored in :ref:`swap-example`, but for tutorial
purposes, such optimizations are not so important. This implementation is
covered in part after the exploration of the Cryptol specification.

A SAWScript containing the specifications of memory layouts and orchestration
of the verification itself ties everything together. This will be covered last,
including some performance comparisons between compositional and
non-compositional verification.


A Cryptol Specification
-----------------------

The Cryptol specification in ``examples/salsa20/salsa20.cry`` implements
directly the functions defined in Bernstein's paper linked above. Because there
is so much code, this section will only go through some of the functions in
detail, in order to highlight some features of Cryptol.

The first function defined is ``quarterround : [4][32] -> [4][32]``: It takes
a sequence of 4 32-bit words and performs the operation defined in Bernstein's
specification; note that the Cryptol code resembles the mathematics very
closely, including the use of the Cryptol operator ``<<<`` which performs a
left rotation on a sequence:

.. literalinclude:: examples/salsa20/Salsa20.cry
  :language: Cryptol
  :start-after: // BEGIN QUARTERROUND
  :end-before: // END QUARTERROUND

``quarterround`` is used in the definition of two other functions, ``rowround``
and ``columnround``, which perform the operation on the rows and columns of a
particular matrix, represented as a flat sequence of 16 32-bit words.

These two operations are composed (``rowround`` after ``columnround``) to form
the ``doubleround`` operation. The Cryptol code for this composition is exactly
as that which appears in Bernstein's specification document:

.. literalinclude:: examples/salsa20/Salsa20.cry
  :language: Cryptol
  :start-after: // BEGIN DOUBLEROUND
  :end-before: // END DOUBLEROUND

Combined with some utility functions for mapping sequences of four bytes to and
from little-endian 32-bit words, ``doubleround`` gives us the Salsa20 hash
function:

.. literalinclude:: examples/salsa20/Salsa20.cry
  :language: Cryptol
  :start-after: // BEGIN SALSA20
  :end-before: // END SALSA20

This particular function is notable for its use of *laziness*: The definition
of the variable ``zs`` in the ``where`` clause is given as a sequence
comprehension referring to ``zs`` itself; this results in an infinite sequence
which will only be evaluated *as needed*. More information about lazy
evaluation can be found `in this excellent Medium article <https://medium.com/background-thread/what-is-lazy-evaluation-programming-word-of-the-day-8a6f4410053f>`_.

The next function, ``Salsa20_expansion``, demonstrates a unique feature of
Cryptol's type system: Arithmetic predicates. Part of the type is
``{a} (a >= 1, 2 >= a) => ...``, which says that ``a`` is a type variable
which can only take numeric values 1 and 2. This allwed this function to be
written to be polymorphic over the allowed key sizes, namely 16- and 32-bit.
Note the behavior in the definition that is conditioned on the value of ``a``:

.. literalinclude:: examples/salsa20/Salsa20.cry
  :language: Cryptol
  :start-after: // BEGIN SALSA20_EXPANSION
  :end-before: // END SALSA20_EXPANSION

The key expansion function finally allows for the encryption function to be
implemented. Note the further use of arithmetic predicates and laziness:

.. literalinclude:: examples/salsa20/Salsa20.cry
  :language: Cryptol
  :start-after: // BEGIN SALSA20_ENCRYPT
  :end-before: // END SALSA20_ENCRYPT

The complete implementation can be visualized at a high-level using the
following diagram:

TODO: Insert boxes/arrows showing data flow for Salsa20

A C Implementation to Verify
----------------------------

TODO: Walk through the C implementation of Salsa20

.. literalinclude:: examples/salsa20/salsa20.c
  :language: C
  :start-after: // BEGIN ROTL
  :end-before: // END ROTL

.. literalinclude:: examples/salsa20/salsa20.c
  :language: C
  :start-after: // BEGIN QUARTERROUND
  :end-before: // END QUARTERROUND

.. literalinclude:: examples/salsa20/salsa20.c
  :language: C
  :start-after: // BEGIN ROWROUND
  :end-before: // END ROWROUND

.. literalinclude:: examples/salsa20/salsa20.c
  :language: C
  :start-after: // BEGIN COLUMNROUND
  :end-before: // END COLUMNROUND

.. literalinclude:: examples/salsa20/salsa20.c
  :language: C
  :start-after: // BEGIN DOUBLEROUND
  :end-before: // END DOUBLEROUND

.. literalinclude:: examples/salsa20/salsa20.c
  :language: C
  :start-after: // BEGIN LITTLEENDIAN
  :end-before: // END LITTLEENDIAN

.. literalinclude:: examples/salsa20/salsa20.c
  :language: C
  :start-after: // BEGIN LITTLEENDIAN_INVERSE
  :end-before: // END LITTLEENDIAN_INVERSE

.. literalinclude:: examples/salsa20/salsa20.c
  :language: C
  :start-after: // BEGIN SALSA20
  :end-before: // END SALSA20

.. literalinclude:: examples/salsa20/salsa20.c
  :language: C
  :start-after: // BEGIN SALSA20_EXPANSION16
  :end-before: // END SALSA20_EXPANSION16

.. literalinclude:: examples/salsa20/salsa20.c
  :language: C
  :start-after: // BEGIN SALSA20_EXPANSION32
  :end-before: // END SALSA20_EXPANSION32

.. literalinclude:: examples/salsa20/salsa20.c
  :language: C
  :start-after: // BEGIN SALSA20_ENCRYPT
  :end-before: // END SALSA20_ENCRYPT

Specification and Verification
------------------------------

TODO: Walk through the SAWScript specs/verifications

.. literalinclude:: examples/salsa20/salsa20.saw
  :language: Cryptol
  :start-after: // BEGIN ALLOC_INIT
  :end-before: // END ALLOC_INIT

.. literalinclude:: examples/salsa20/salsa20.saw
  :language: Cryptol
  :start-after: // BEGIN ALLOC_INIT_READONLY
  :end-before: // END ALLOC_INIT_READONLY

.. literalinclude:: examples/salsa20/salsa20.saw
  :language: Cryptol
  :start-after: // BEGIN PTR_TO_FRESH
  :end-before: // END PTR_TO_FRESH

.. literalinclude:: examples/salsa20/salsa20.saw
  :language: Cryptol
  :start-after: // BEGIN PTR_TO_FRESH_READONLY
  :end-before: // END PTR_TO_FRESH_READONLY

.. literalinclude:: examples/salsa20/salsa20.saw
  :language: Cryptol
  :start-after: // BEGIN ONEPTR_UPDATE
  :end-before: // END ONEPTR_UPDATE

.. literalinclude:: examples/salsa20/salsa20.saw
  :language: Cryptol
  :start-after: // BEGIN QUARTERROUND
  :end-before: // END QUARTERROUND

.. literalinclude:: examples/salsa20/salsa20.saw
  :language: Cryptol
  :start-after: // BEGIN ROWROUND
  :end-before: // END ROWROUND

.. literalinclude:: examples/salsa20/salsa20.saw
  :language: Cryptol
  :start-after: // BEGIN COLUMNROUND
  :end-before: // END COLUMNROUND

.. literalinclude:: examples/salsa20/salsa20.saw
  :language: Cryptol
  :start-after: // BEGIN DOUBLEROUND
  :end-before: // END DOUBLEROUND

.. literalinclude:: examples/salsa20/salsa20.saw
  :language: Cryptol
  :start-after: // BEGIN SALSA20
  :end-before: // END SALSA20

.. literalinclude:: examples/salsa20/salsa20.saw
  :language: Cryptol
  :start-after: // BEGIN SALSA20_EXPANSION
  :end-before: // END SALSA20_EXPANSION

.. literalinclude:: examples/salsa20/salsa20.saw
  :language: Cryptol
  :start-after: // BEGIN SALSA20_ENCRYPT
  :end-before: // END SALSA20_ENCRYPT

.. literalinclude:: examples/salsa20/salsa20.saw
  :language: Cryptol
  :start-after: // BEGIN MAIN
  :end-before: // END MAIN

.. rubric:: Footnotes

.. [#salsa20wiki] https://en.wikipedia.org/wiki/Salsa20
