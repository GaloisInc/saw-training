Simple Proof Maintenance: HMAC
==============================

As code evolves, specifications of it and proofs of those specifications evolve
as well. This section covers *proof maintenance*: How, after some changes to a
piece of code, specifications and SAW proof scripts must be updated to reflect
those changes. We use a real example of necessary proof maintenance taken from
the HMAC implementation in the Amazon s2n repository.

Key concepts:

* Proof maintenance

  - Changes to code which has been verified with respect to some specification
    often demand updates to that same specification and proof.
  - Complexity of changes to specification/proof script roughly correlate with
    the complexity of changes to the underlying software system being verified.

* SAW/Cryptol practice

HMAC Implementation and Verification: Pre-Update
------------------------------------------------

The HMAC implementation

The HMAC Update
---------------

Updating the Specification and Proof
------------------------------------
