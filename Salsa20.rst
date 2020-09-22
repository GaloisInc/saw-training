Compositional Verification and Salsa20
======================================

.. index:: compositional verification

:ref:`swap-example` demonstrates verification and maintenance for a
small standalone function. Most interesting programs are not just
single functions, however. Good software engineering practice entails
splitting programs into smaller functions, each of which can be
understood and tested independently. Compositional verification in SAW
allows this structure to be reflected in proofs as well, so that each
function can be verified independently. This can greatly increase the
performance of a verification script.

This section describes the verification of an implementation of the
Salsa20 encryption algorithm. Complete example code can be found in
the ``examples/salsa20`` of :download:`the example code </examples.tar.gz>`.


Salsa20 Verification Overview
-----------------------------

Salsa20 is a stream cipher developed in 2005 by Daniel J. Bernstein, built on a
pseudorandom function utilizing add-rotate-XOR (ARX) operations on 32-bit words
[#salsa20wiki]_. His original specification can be found
`here <http://cr.yp.to/snuffle/spec.pdf>`_.

The specification for this task is a trusted implementation written in
:term:`Cryptol`. This is analogous to what is covered in :ref:`swap-cryptol` in
the ``swap`` example, but for a larger system. Some examples from this
specification are explored below for the sake of showing what large-scale
Cryptol programs look like.

.. DTC: Is it actually true that there should be extra hacks/optimizations? From where do we know that?

.. DTC: The xor-swap is not actually a thing that makes programs faster or better - these days it's a historical curiosity

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

The Cryptol specification in ``examples/salsa20/salsa20.cry`` directly
implements the functions defined in Bernstein's
`specification`_. Because there is so much code, this section will
only go through some of the functions in detail, in order to highlight
some features of Cryptol.

.. _specification: http://cr.yp.to/snuffle/spec.pdf

The first example function is ``quarterround``. Its type is ``[4][32]
-> [4][32]``, which means that it is a function that maps sequences of
four 32-bit words into sequences of four 32-bit words. The ``[y0, y1,
y2, y3]`` notation is pattern matching that pulls apart the four
elements of the input sequence, naming each 32-bit word. The Cryptol
operator ``<<<`` performs a left rotation on a sequence.

.. literalinclude:: examples/salsa20/Salsa20.cry
  :language: Cryptol
  :start-after: // BEGIN QUARTERROUND
  :end-before: // END QUARTERROUND

This Cryptol code closely resembles the definition in Section 3 of the
specification. This The definition reads:

If :math:`y = (y_0, y_1, y_2, y_3)` then :math:`\mathrm{quarterround(y) = (z_0, z_1, z_2, z_3)}` where

.. math::
  \begin{array}{rcl}
  z_1 & = & y_1 \oplus{} ((y_0 + y_3) <\!\!<\!\!< 7)\\
  z_2 & = & y_2 \oplus{} ((z_1 + y_0) <\!\!<\!\!< 9)\\
  z_3 & = & y_3 \oplus{} ((z_2 + z_1) <\!\!<\!\!< 13)\\
  z_0 & = & y_0 \oplus{} ((z_3 + z_2) <\!\!<\!\!< 18)\\
  \end{array}

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

The C Implementation to Verify
------------------------------

As stated above, the C implementation is very similar to its specification, the
most obvious difference being heavy use of in-place mutation and the encryption
function itself being restricted to 32-bit keys. For this reason, only a
couple of the functions are explicated in detail (and it is these functions
whose SAWScript specifications will be studied later).

The ``s20_quarterround`` shows the most important deviations from the
specification code, namely the use of in-place mutation rather than the
functional paradigm of Cryptol:

.. literalinclude:: examples/salsa20/salsa20.c
  :language: C
  :start-after: // BEGIN QUARTERROUND
  :end-before: // END QUARTERROUND

This function directly modifies the targets of its argument pointers, which
should be contrasted with the building/returning of a new sequence in Cryptol.
This paradigm shift will be visible in the SAWScript specification, as that is
where the memory management of the C is connected to the pure computation of
the Cryptol.

The other function of note is the Salsa20 hash. The function ``s20_hash``
explicitly performs the iterations of the lower-level functions, in contrast to
the use of sequence comprehensions and laziness in the Cryptol implementation.
Arguably, this code is clearer than the correspondent Cryptol, but this is
certainly more of a matter of taste than anything:

.. literalinclude:: examples/salsa20/salsa20.c
  :language: C
  :start-after: // BEGIN SALSA20
  :end-before: // END SALSA20

Again note the pervasive use of in-place mutation: As with ``s20_quarterround``
and the other functions necessary to implement Salsa20, this will be connected
to the pure computations in the Cryptol specification via the SAWScript
specification which details memory layouts.

A diagrammatic view of the C implementation is less informative than that for
Cryptol due in large part to the pervasive use of in-place mutation, so none is
provided here.


SAW Specification and Verification
----------------------------------

The SAW specification for this Salsa20 implementation is comprised of a couple
of convenient helper functions, a specification for each of the interesting
functions in the Salsa20 specification (i.e. the functions detailed in
Bernstein's specification document), and a ``main : TopLevel ()`` which
performs the actual verification.

First, one of the helper functions. A number of the functions to verify all
have the same shape: They take a single pointer and update the value pointed to
according to some Cryptol function. This pattern is abstracted out to the
SAWScript function ``oneptr_update_func n ty f``, which takes ``n : String``
naming the parameter for pretty-printing,  ``ty : LLVMType`` describing the
parameter type, and ``f : Term`` to apply to the parameter:

.. literalinclude:: examples/salsa20/salsa20_compositional.saw
  :language: Cryptol
  :start-after: // BEGIN ONEPTR_UPDATE
  :end-before: // END ONEPTR_UPDATE

This helper function is sufficient to build the specfications for many of the
functions the implementation of Salsa20 depends on.

These functions are all dependent on ``s20_quarterround``, though, so that
function also needs a specification:

.. literalinclude:: examples/salsa20/salsa20_compositional.saw
  :language: Cryptol
  :start-after: // BEGIN QUARTERROUND
  :end-before: // END QUARTERROUND

The helper function ``ptr_to_fresh`` allocates space for a new symbolic
variable of the given type, returning both the symbol and the pointer. The
symbols are passed to the Cryptol function ``quarterround`` to compute the
expected result values, which are then used as the expected targets of the
pointers in the post-condition of the SAW specification.

The specification for ``s20_hash`` is one for which ``oneptr_update_func`` is
sufficient:

.. literalinclude:: examples/salsa20/salsa20_compositional.saw
  :language: Cryptol
  :start-after: // BEGIN SALSA20
  :end-before: // END SALSA20

Putting everything together, ``main`` will verify the implementation functions
according to these specifications. This is where the use of compositional
verification is realized: In calls to ``crucible_llvm_verify``, it is possible
to specify a list of ``CrucibleMethodSpec`` which come from earlier
verifications. Notice in the highlighted lines that the results of earlier
verifications within ``main`` are passed along:

.. literalinclude:: examples/salsa20/salsa20_compositional.saw
  :language: Cryptol
  :start-after: // BEGIN MAIN
  :end-before: // END MAIN
  :emphasize-lines: 4-11

In effect, threading these earlier verification results through tells SAW
"if you see these functions, no need to go deeper: they're all good." This same
mechanism is used to specify facts that should be assumed during verification.

Other things to note about this particular example not related to the use of
compositional verification are:

1. The use of the path-satisfiability parameter to verify functions containing
   loops that aren't 'obviously' bounded (i.e. bounded by a constant.)
2. The lack of verification of the 16-bit version of Salsa20
3. The necessity of verifying the encryption for particular message sizes

To 1: The details of path-satisfiability are beyond the scope of this tutorial,
but in essence, this must be turned on when code contains loops that are
bounded, but not by a constant. SAW does not yet fully support verification of
code containing potentially unbounded loops.

To 2: The C implementation being verified does not provide the 16-bit version
of the encryption algorithm itself, hence it's missing from the SAW code.

To 3: This is related to 1, in that it is an inherent limitation of SAW that
functions accepting arbitrarily sized inputs cannot be verified except on
particluar instantiations of that input size. In other words, while Salsa20
can operate on any input size, SAW can only operate on finite programs.


Comparing Compositional and Non-compositional Verification
----------------------------------------------------------

A natural question is, "what happens when you *don't* use compositional
verification?"


.. rubric:: Footnotes

.. [#salsa20wiki] https://en.wikipedia.org/wiki/Salsa20
