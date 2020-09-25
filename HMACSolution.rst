Maintaining Proofs: s2n HMAC
============================

As mentioned in :ref:`swap-code-evolution`, the evolution of a program
is accompanied by the evolution of its specifications. A key part of
using SAW and Cryptol to verify a software system is the ongoing
maintenance of proof artifacts through the software development
lifecycle.

.. index::
  single: proof; maintenance
  single: maintenance; proof

*Proof maintenance* is the process of preserving the correspondence between a program, its specification, and its
proof of correctness as requirements change over time. This section describes a real-world example of proof
maintenance, adapted from `these changes to the HMAC implementation in Amazon's s2n <https://github.com/awslabs/s2n/commit/e283d76f966828f27002dea7c7c0bd9865fea926>`_.
The code's file structure has been reorganized slightly, but the code itself is untouched.


This task will be approached as if the changes to the implementation are
given, and the goal will be to evolve the relevant specifications to
match. Throughout, take note of the very direct correspondence between the
changes to the code and the changes to the specifications - it will help to
follow along in the provided ``examples/hmac`` directory.


The Updates to the Implementation
---------------------------------

This section provides an overview of the changes to the implementation that
demand some level of proof maintenance.

The s2n HMAC implementation needed to be updated to make use of an additional
piece of hashing state, ``outer_just_key``, for the implementation of TLS.
At its core, this change is captured by the addition of
a new field to the ``s2n_hmac_state`` structure as it is defined in
``s2n_hmac_old.h``. The resulting structure looks like this:

.. literalinclude:: examples/hmac/s2n_hmac_new.h
  :language: C
  :caption: The updated definition of ``struct s2n_hmac_state``
  :start-after: // BEGIN S2N_HMAC_STATE_STRUCT
  :end-before: // END S2N_HMAC_STATE_STRUCT
  :emphasize-lines: 12

The addition of this new field saw corresponding changes to the implementation
code, which can be found in :download:`s2n_hmac_new.c </downloads/examples/hmac/s2n_hmac_new.c>`. These changes included memory
allocations, initializations, updates, and frees. The following code sample
gives a good sense of the types of changes involved:

.. literalinclude:: examples/hmac/s2n_hmac_new.c
  :language: C
  :caption: A sample of the changes introduced in the referenced commit
  :start-after: // BEGIN S2N_TLS_HMAC_INIT
  :end-before: // END S2N_TLS_HMAC_INIT
  :emphasize-lines: 9,36-37

The complete diff between ``s2n_hmac_old.c`` and ``s2n_hmac_new.c`` shows a
number of updates similar to that above:

.. literalinclude:: examples/hmac/s2n_hmac_new.c
    :language: C
    :caption: Changes to the s2n HMAC implementation
    :diff: examples/hmac/s2n_hmac_old.c

From these changes alone, the work needed to keep the proofs up-to-date with
the implementation can be very reasonably estimated. In this case, it will be
necessary to complete the following tasks:

* Add the new field to the correct type(s) in the Cryptol reference
  implementation
* Add the relevant implementation details to the function(s) using the changed
  type
* Update the SAWScript to reflect new memory layouts, initializations, etc
  implied by the updated type

Take note of the similarities to the ``rotr3`` example in
:ref:`swap-code-evolution`; these types of update are ubiquitous when working
on proof maintenance tasks.


Updating the Cryptol Specification
----------------------------------

In order for verification to go through, the Cryptol specification (that is,
the implementation we trust as correct) must be updated to reflect the
existence of the new state field introduced above.

The Cryptol type corresponding to the updated state container must,
like the C structure, be augmented with an ``outer_just_key`` field
that has the appropriate type, like so:

.. literalinclude:: examples/hmac/HMAC_iterative_new.cry
  :language: Cryptol
  :caption: The updated definition of ``HMAC_c_state``
  :start-after: // BEGIN HMAC_C_STATE
  :end-before: // END HMAC_C_STATE
  :emphasize-lines: 10

This very clearly corresponds to the change to the ``s2n_hmac_state`` structure
described above, other than the specialization to SHA512 (the C implementation
is abstracted over the chosen hashing algorithm.)

As above, here is a sample of how the functions that use the
``HMAC_c_state`` type must change:

.. literalinclude:: examples/hmac/HMAC_iterative_new.cry
  :language: Cryptol
  :start-after: // BEGIN HMAC_INIT_C_STATE
  :end-before: // END HMAC_INIT_C_STATE
  :emphasize-lines: 24,40-42

Take note of how similar these changes are to those in the analogous C code
shown above; this is true more generally, as can be seen in the complete
diff between ``HMAC_iterative_old.cry`` and ``HMAC_iterative_new.cry``:

.. literalinclude:: examples/hmac/HMAC_iterative_new.cry
  :language: Cryptol
  :caption: Changes to the HMAC Cryptol specification
  :diff: examples/hmac/HMAC_iterative_old.cry


Updating the SAW Specifications
-------------------------------

The final step to proof maintenance is updating the SAW portion of the
specification. This can range in difficulty from simply updating
memory layouts to changing what the specification actually asserts
about the program. For the HMAC updates, the necessary changes are closer
to the former rather than the latter, since the implementation change was
the addition of a data field rather than overall changes to the control flow.

A reliable strategy for updating ``HMAC_old.saw`` to account for
``outer_just_key`` is a simple search for the names of other fields already
present in the structure; these will likely appear where memory layouts and
initializations that need to be augmented are specified.

Indeed, such a search for the term "outer" in ``HMAC_old.saw`` reveals not only
where memory layouts are specified, but embedded Cryptol terms of the type
adjusted in the previous section. One of the memory layout specifications
found through this search looks like this:

.. literalinclude:: /examples/hmac/HMAC_old.saw
  :language: SAWScript
  :caption: Example SAWScript memory layout to update
  :start-after: // BEGIN HMAC_MEM_LAYOUT_OLD
  :end-before: // END HMAC_MEM_LAYOUT_OLD
  :emphasize-lines: 9

Another improvement that can be made to this code is to use the
``crucible_field`` primitive instead of ``crucible_elem``, which
allows reference to structure fields by name rather than by
index. This, and the necessary change to memory layout, appear below.

.. literalinclude:: /examples/hmac/HMAC_new.saw
  :language: SAWScript
  :caption: Updated memory layouts including ``outer_just_key``
  :start-after: // BEGIN HMAC_MEM_LAYOUT_NEW
  :end-before: // END HMAC_MEM_LAYOUT_NEW
  :emphasize-lines: 9

The other change necessary is the aforementioned update to embedded Cryptol
terms using the ``HMAC_c_state`` type augmented in the previous section. The
original code found by searching looks like this:

.. literalinclude:: /examples/hmac/HMAC_old.saw
  :language: SAWScript
  :caption: Example embedded Cryptol to update
  :start-after: // BEGIN HMAC_CRYPTOL_OLD
  :end-before: // END HMAC_CRYPTOL_OLD
  :emphasize-lines: 10

And the update corresponds exactly to the one in the Cryptol reference
implementation:

.. literalinclude:: /examples/hmac/HMAC_new.saw
 :language: SAWScript
 :caption: Updated embedded Cryptol including ``outer_just_key``
 :start-after: // BEGIN HMAC_CRYPTOL_NEW
 :end-before: // END HMAC_CRYPTOL_NEW
 :emphasize-lines: 11

The complete set of changes to the SAW specification can be seen in the diff
between ``HMAC_old.saw`` and ``HMAC_new.saw``:

.. literalinclude:: /examples/hmac/HMAC_new.saw
  :language: SAWScript
  :caption: Changes to the HMAC SAW specification
  :diff: /examples/hmac/HMAC_old.saw

With this, the specifications have been updated to account for the changes to
the implementation, and verification via SAW will go through as intended.
