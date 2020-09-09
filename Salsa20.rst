Compositional Verification and Salsa20
======================================

This is another complete verification example, focusing on the use of
*compositional verification* for more complex software systems. In particular,
we verify the Salsa20 encryption algorithm.

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

Specification and Verification
------------------------------

TODO: Walk through the SAWScript specs/verifications
