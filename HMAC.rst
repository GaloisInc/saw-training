Maintaining Proofs: s2n HMAC
============================

As mentioned in :ref:`swap-code-evolution`, the evolution of a program is
accompanied by the evolution of its specifications. The maintenance of a tight
correspondence between a software system, its specifications, and the proofs
that these specifications are satisfied can be accomplished fairly easily with
SAW and Cryptol.

The tasks necessary to keep a program and the verifications performed on it
consistent fall under the umbrella of *proof maintenance*. This section shows
proof maintenance on a real example, adapted from `these changes to the HMAC
implementation underlying Amazon's s2n <https://github.com/awslabs/s2n/commit/e283d76f966828f27002dea7c7c0bd9865fea926>`_.
Note that the code has been modified slightly to better suit this tutorial.

This task will be approached as if the changes to the 'real' implementation are
given, and the goal will be to evolve the relevant specifications to
match. Throughout, take note of the very direct correspondence between the
changes to the code and the changes to the specifications - it will help to
follow along yourself in the provided ``examples/hmac`` directory.


s2n HMAC Summary
----------------

Given the size of the s2n HMAC implementation, it is impractical to go through
all of it in detail. The most relevant files are listed below, with brief
descriptions of their contents:

TODO: Def list of files

All other files are headers/modules these relevant files are dependent on and
will not be discussed. The curious reader can explore them further by following
the above link to the commit which inspired this tutorial.

Take note of the suffixes ``_old`` and ``_new``: These make obvious what files
needed to be changed as part of proof maintenance, and allow for one to easily
study the changes made via tools like ``diff``. Taking the time to do this is
encouraged in order to develop a more complete understanding of the maintenance
task than the samples chosen for this written tutorial can offer.


The Updates to the 'Real' Implementation
----------------------------------------

This section provides an overview of the changes to the real implementation
that demand some level of proof maintenance.

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
code, which can be found in ``s2n_hmac_new.c``. These changes included memory
allocations, initializations, updates, and frees. The following code sample
gives a good sense of the types of changes involved:

.. literalinclude:: examples/hmac/s2n_hmac_new.c
  :language: C
  :caption: A sample of the changes introduced in the referenced commit
  :start-after: // BEGIN S2N_TLS_HMAC_INIT
  :end-before: // END S2N_TLS_HMAC_INIT
  :emphasize-lines: 9,36-37

Similar updates can be found throughout the code; you're encouraged to explore
the diff between ``s2n_hmac_old.c`` and ``s2n_hmac_new.c`` to have a better
understanding of all of the changes implemented.

More interesting, though, is the fact that knowing these changes, it is
possible to estimate reasonably what will need to change among reference
implementations, specifications, and proof scripts verifying that these
specifications are met. In this case, one might guess that it will be
necessary to complete the following tasks as proof maintenance:

* Add the new field to the correct type(s) in the Cryptol reference
  implementation
* Add the relevant implementation details to the function(s) using the changed
  type
* Update the SAWScript to reflect new memory layouts, initializations, etc
  implied by the updated type

Indeed, these expected next steps are precisely how the verification of s2n
HMAC must evolve to account for the code updates.


Updating the Reference Implementation
-------------------------------------

In order for verification to go through, the reference implementation (that is,
the implementation we trust as correct) must be updated to reflect the
existence of the new state field introduced above. Due to the close-to-the-math
nature of Cryptol, this should be a small enough change that faith in the
correctness of the reference is not lost.

The Cryptol type corresponding to the updated state container must, like the C
structure, be augmented with an ``outer_just_key`` field of appropriate type,
like so:

.. literalinclude:: examples/hmac/HMAC_iterative_new.cry
  :language: Cryptol
  :caption: The updated definition of ``HMAC_c_state``
  :start-after: // BEGIN HMAC_C_STATE
  :end-before: // END HMAC_C_STATE
  :emphasize-lines: 10

This very clearly corresponds to the change to the ``s2n_hmac_state`` structure
described above, other than the specialization to a particular hashing
algorithm.

As above, here is a sample of how the functions making use of the
``HMAC_c_state`` type must change:

.. literalinclude:: examples/hmac/HMAC_iterative_new.cry
  :language: Cryptol
  :start-after: // BEGIN HMAC_INIT_C_STATE
  :end-before: // END HMAC_INIT_C_STATE
  :emphasize-lines: 24,40-42

Take note of how similar these changes are to those in the analogous C code
shown above; this is true more generally, and you are encouraged to explore
this further in the diff between ``HMAC_iterative_old.cry`` and
``HMAC_iterative_new.cry``, paying special attention to how these updates
relate to those between ``s2n_hmac_old.c`` and ``s2n_hmac_new.c``.


Updating the Specifications
---------------------------

The final step to proof maintenance is updating the specifications. This task
can range in difficulty from simply updating memory layouts to changing what
the specification actually asserts about the program; in this example, the
necessary changes are closer to the former rather than latter, since the
implementation change was the straightforward addition of a data field.

A reliable strategy for updating ``HMAC_old.saw`` to account for
``outer_just_key`` is a simple search for the names of other fields already
present in the structure; these will likely appear where memory layouts and
initializations that need to be augmented are specified.

Indeed, such a search for the term 'outer' in ``HMAC_old.saw`` reveals not only
where memory layouts are specified, but embedded Cryptol terms of the type
adjusted in the previous section. One of the memory layout specifications
found through this search looks like this:

.. literalinclude:: /examples/hmac/HMAC_old.saw
  :language: SAWScript
  :caption: Example SAWScript memory layout to update
  :start-after: // BEGIN HMAC_MEM_LAYOUT_OLD
  :end-before: // END HMAC_MEM_LAYOUT_OLD
  :emphasize-lines: 9

Sharp-eyed readers may see that there is another improvement that can be made
to this code: Use of the ``crucible_field`` primitive rather than
``crucible_elem``, which allows reference to structure fields by name rather
than by index. This, and the necessary change to memory layout appear below.

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

And the update corresponds exactly to that done in the Cryptol reference
implementation:

.. literalinclude:: /examples/hmac/HMAC_new.saw
 :language: SAWScript
 :caption: Updated embedded Cryptol including ``outer_just_key``
 :start-after: // BEGIN HMAC_CRYPTOL_NEW
 :end-before: // END HMAC_CRYPTOL_NEW
 :emphasize-lines: 11

With this (and the analogous changes not shown here for brevity), the
specifications have been updated to account for the changes to the
implementation. As before, you are encouraged to look at the diff between
``HMAC_old.saw`` and ``HMAC_new.saw`` to develop a more complete understanding
of this example and what proof maintenance looks like more generally.
