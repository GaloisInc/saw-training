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
proof of correctness as requirements change over time. This section poses as an exercise an extended proof-maintenance
task, adapted from `these changes to the HMAC implementation in Amazon's s2n <https://github.com/awslabs/s2n/commit/e283d76f966828f27002dea7c7c0bd9865fea926>`_.
The code's file structure has been reorganized slightly, but the code itself is untouched.


This task will be approached as if the changes to the implementation are
given, and the goal will be to evolve the relevant specifications to
match. While completing the exercises, take note of the correspondences between
the changes to the code and the changes to the specifications.


Background: The Updates to the Implementation
------------------------------------------

This section provides an overview of the changes to the implementation that
form the basis of the proof maintenance task to be completed.

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
on proof maintenance tasks. It will help to review that section before
completing these exercises.


Exercise: Update the Cryptol Specification
------------------------------------------

In order for verification to go through, the Cryptol specification (that is,
the implementation trusted to be correct) must be updated to reflect the
existence of the new state field introduced above.

Your task is to perform these updates in
:download:`HMAC_iterative_old.cry </downloads/examples/hmac/HMAC_iterative_old.cry>`.
Use the bullet points above as a rough guide, and if you get stuck, there is a
complete solution presented on the next page.


Exercise: Update the SAW Specifications
---------------------------------------

The final step to proof maintenance is updating the SAW portion of the
specification. This can range in difficulty from simply updating
memory layouts to changing what the specification actually asserts
about the program. For the HMAC updates, the necessary changes are closer
to the former rather than the latter, since the implementation change was
the addition of a data field rather than overall changes to the control flow.

In this exercise, you will edit the file
:download:`HMAC_old.saw </downloads/examples/hmac/HMAC_old.saw>` to add the
memory layout information for the state field added to the C implementation.
Hint: A reliable strategy for updating ``HMAC_old.saw`` to account for
``outer_just_key`` is a simple search for the names of other fields already
present in the structure; these will likely appear where memory layouts and
initializations that need to be augmented are specified.

As before, if you get stuck, there is a complete solution presented on the next
page.
