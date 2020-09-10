Compositional Verification and Salsa20
======================================

This is another complete verification example, focusing on the use of
*compositional verification* for more complex software systems. In particular,
we verify the Salsa20 encryption algorithm, which reflects what a complete
verification task may look like in practice.

Key concepts:

* Compositional verification

  - Specification-based approaches to verification allow for compositional
    reasoning: We only need to prove any particular result once, and can then
    use it freely in future proofs
  - Very large/complex systems that are otherwise too unwieldy to verify can be
    reasoned about using this technique

* Cryptol practice

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
