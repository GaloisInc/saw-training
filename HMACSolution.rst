Proof Maintenance Exercises: Solutions
======================================

This section provides a detailed solution to the two exercises in
:ref:`hmac-problem`.


Updating the Cryptol Specification
----------------------------------

The Cryptol type corresponding to the updated state container must,
like the C structure, be augmented with an ``outer_just_key`` field
that has the appropriate type, like so:

.. literalinclude:: examples/hmac/HMAC_iterative_new.cry
  :language: Cryptol
  :start-after: // BEGIN HMAC_C_STATE
  :end-before: // END HMAC_C_STATE

This very clearly corresponds to the change to the ``s2n_hmac_state`` structure
in the C implementation, other than the specialization to SHA512. In the C
implementation, the code is abstracted over the chosen hashing algorithm.

Here is a sample of how the functions that use the ``HMAC_c_state`` type must
change:

.. literalinclude:: examples/hmac/HMAC_iterative_new.cry
  :language: Cryptol
  :start-after: // BEGIN HMAC_INIT_C_STATE
  :end-before: // END HMAC_INIT_C_STATE

Take note of how similar these changes are to those in the analogous C code;
this is true more generally, as can be seen in the complete diff between
``HMAC_iterative_old.cry`` and ``HMAC_iterative_new.cry``:

.. literalinclude:: examples/hmac/HMAC_iterative_new.cry
  :language: Cryptol
  :diff: downloads/examples/hmac/HMAC_iterative_old.cry


Updating the SAW Specifications
-------------------------------

Using the hint given in the exercise, a search for the term "outer" in
``HMAC_old.saw`` reveals not only where memory layouts are specified, but
embedded Cryptol terms of the type adjusted in the previous section. One of the
memory layout specifications found through this search looks like this:

.. literalinclude:: /examples/hmac/HMAC_old.saw
  :language: SAWScript
  :start-after: // BEGIN HMAC_MEM_LAYOUT_OLD
  :end-before: // END HMAC_MEM_LAYOUT_OLD

Another improvement that can be made to this code is to use the
``crucible_field`` primitive instead of ``crucible_elem``, which
allows reference to structure fields by name rather than by
index. This, and the necessary change to memory layout, appear below.

.. literalinclude:: /examples/hmac/HMAC_new.saw
  :language: SAWScript
  :start-after: // BEGIN HMAC_MEM_LAYOUT_NEW
  :end-before: // END HMAC_MEM_LAYOUT_NEW

The other change necessary is the aforementioned update to embedded Cryptol
terms using the ``HMAC_c_state`` type augmented in the previous section. The
original code found by searching looks like this:

.. literalinclude:: /examples/hmac/HMAC_old.saw
  :language: SAWScript
  :start-after: // BEGIN HMAC_CRYPTOL_OLD
  :end-before: // END HMAC_CRYPTOL_OLD

And the update corresponds exactly to the one in the Cryptol specification:

.. literalinclude:: /examples/hmac/HMAC_new.saw
 :language: SAWScript
 :start-after: // BEGIN HMAC_CRYPTOL_NEW
 :end-before: // END HMAC_CRYPTOL_NEW

The complete set of changes to the SAW specification can be seen in the diff
between ``HMAC_old.saw`` and ``HMAC_new.saw``:

.. literalinclude:: /examples/hmac/HMAC_new.saw
  :language: SAWScript
  :diff: downloads/examples/hmac/HMAC_old.saw

With this, the specifications have been updated to account for the changes to
the implementation, and verification via SAW will go through as intended.
