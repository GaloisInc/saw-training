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


Salsa20
-------

TODO: Describe Salsa20 at a high-level

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
