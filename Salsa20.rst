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


A Cryptol Reference Implementation
----------------------------------

TODO: Walk through the Cryptol implementation of Salsa20

.. literalinclude:: examples/salsa20/Salsa20.cry
  :language: Cryptol
  :start-after: // BEGIN QUARTERROUND
  :end-before: // END QUARTERROUND

.. literalinclude:: examples/salsa20/Salsa20.cry
  :language: Cryptol
  :start-after: // BEGIN ROWROUND
  :end-before: // END ROWROUND

.. literalinclude:: examples/salsa20/Salsa20.cry
  :language: Cryptol
  :start-after: // BEGIN COLUMNROUND
  :end-before: // END COLUMNROUND

.. literalinclude:: examples/salsa20/Salsa20.cry
  :language: Cryptol
  :start-after: // BEGIN DOUBLEROUND
  :end-before: // END DOUBLEROUND

.. literalinclude:: examples/salsa20/Salsa20.cry
  :language: Cryptol
  :start-after: // BEGIN LITTLEENDIAN
  :end-before: // END LITTLEENDIAN

.. literalinclude:: examples/salsa20/Salsa20.cry
  :language: Cryptol
  :start-after: // BEGIN LITTLEENDIAN_INVERSE
  :end-before: // END LITTLEENDIAN_INVERSE

.. literalinclude:: examples/salsa20/Salsa20.cry
  :language: Cryptol
  :start-after: // BEGIN SALSA20
  :end-before: // END SALSA20

.. literalinclude:: examples/salsa20/Salsa20.cry
  :language: Cryptol
  :start-after: // BEGIN SALSA20_EXPANSION
  :end-before: // END SALSA20_EXPANSION

.. literalinclude:: examples/salsa20/Salsa20.cry
  :language: Cryptol
  :start-after: // BEGIN SALSA20_ENCRYPT
  :end-before: // END SALSA20_ENCRYPT

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
