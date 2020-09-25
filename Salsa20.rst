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

The implementation to be verified is written in C to closely match the Cryptol
specification. This implementation is shown in part alongside the specification
for comparison purposes.

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
specification. The definition reads:

If :math:`y = (y_0, y_1, y_2, y_3)` then :math:`\mathrm{quarterround(y) = (z_0, z_1, z_2, z_3)}` where

.. math::
  \begin{array}{rcl}
  z_1 & = & y_1 \oplus{} ((y_0 + y_3) <\!\!<\!\!< 7)\\
  z_2 & = & y_2 \oplus{} ((z_1 + y_0) <\!\!<\!\!< 9)\\
  z_3 & = & y_3 \oplus{} ((z_2 + z_1) <\!\!<\!\!< 13)\\
  z_0 & = & y_0 \oplus{} ((z_3 + z_2) <\!\!<\!\!< 18)\\
  \end{array}

Contrast this with the C implementation of ``s20_quarterround``, which makes
heavy use of in-place mutation rather than the functional paradigm of building
and returning a new sequence:

.. literalinclude:: examples/salsa20/salsa20.c
  :language: C
  :start-after: // BEGIN QUARTERROUND
  :end-before: // END QUARTERROUND

This function directly modifies the targets of its argument pointers, a shift
in paradigm that will be highlighted by the SAW specification since that is
where the memory management of the C is connected to the pure computation of
the Cryptol.

``quarterround`` is used in the definition of two other functions, ``rowround``
and ``columnround``, which perform the operation on the rows and columns of a
particular matrix, represented as a flat sequence of 16 32-bit words.

These two operations are composed (``rowround`` after ``columnround``)
to form the ``doubleround`` operation. The Cryptol code for this
composition closely resembles the definition in the specification:

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

.. index:: sequence comprehension

All three definitions in the ``where`` clause are *sequence
comprehensions*, which are similar to Python's generator expressions
or C#'s LINQ.  A sequence comprehension consists of square brackets
that contain an expression, and then one or more branches. Branches
begin with a vertical bar, and they contain one or more
comma-separated bindings. Each binding is a name, an arrow, and
another sequence.

.. DTC TODO some kind of REPL-looking thing for these examples - the text looks busy

The value of a comprehension with one branch is found by evaluating
the expression for each element of the sequence in the branch, with
the name to the left of the arrow set to the current element. The
value of ``[x + 1 | x <- [1,2,3]]`` is ``[2, 3, 4]``. If there are
multiple bindings in the branch, later bindings are repeated for each
earlier value. So the value of ``[(x + 1, y - 1) | x <- [1,2], y <-
[11, 12]]`` is ``[(2, 10), (2, 11), (3, 10), (3, 11)]``. The value of
a comprehension with multiple branches is found by evaluating each
branch in parallel. The value of ``[(x + 1, y - 1) | x <- [1,2] | y <-
[11,12]]`` is ``[(2, 10), (3, 11)]``.

In the ``where`` clause, the definition of ``xw`` can be read as
"First split ``xs`` into 4-byte words, then combine them in a
little-endian manner to obtain 32-bit words." The specific sizes are
automatically found by Cryptol's type checker.

The definition of ``zs`` is an infinite sequence. It begins with
``xw``, the little-endian reorganization of ``xs`` from the previous
paragraph. The ``#`` operator appends sequences. The rest of the
sequence consists of ``doubleround`` applied to each element of ``zs``
itself. In other words, the second element is found by applying
``doubleround`` to ``xw``, the third by applying ``doubleround`` to
the second, and so forth. Stepping through the evaluation yields this
sequence:

.. code-block:: Cryptol

    [xw] # [ doubleround zi | zi <- zs ]

    [xw] # [ doubleround zi | zi <- [xw] # [doubleround zi | zi <- zs] ]

    [xw] # [doubleround xw] # [ doubleround zi | zi <- [doubleround zi | zi <- zs] ]

    [xw] # [doubleround xw] # [ doubleround zi | zi <- [doubleround zi | zi <- [xw] # [doubleround zi | zi <- zs]] ]

    [xw] # [doubleround xw] # [ doubleround zi | zi <- [doubleround xw] # [doubleround zi | zi <- [doubleround zi | zi <- zs]] ]

    [xw] # [doubleround xw] # [doubleround (doubleround xw)] # [ doubleround zi | zi <- [doubleround zi | zi <- [doubleround zi | zi <- zs]] ]

The resulting sequence consists of ``doubleround`` applied :math:`n`
times to ``xw`` at position :math:`n`. This process could, in
principle, continue forever. In Cryptol, however, sequences are
computed lazily, so as long as nothing ever asks for the last element,
the program will still terminate.

The final definition is ``ar``, which adds ``xw`` to the tenth element
of ``zs``, which is the result of applying ``doubleround`` ten times
to ``xw``. In Cryptol, ``+`` is extended over sequences so that adding
two sequences adds their elements. The final result of ``Salsa20`` is
computed by re-joining the split words into the appropriate-sized
sequence.

.. DTC: Do we really want to link to something on Medium, given their shady practices? If so, we should at least credit the author rather than the blog hoster. `in this excellent Medium article <https://medium.com/background-thread/what-is-lazy-evaluation-programming-word-of-the-day-8a6f4410053f>`_.


The C implementation uses in-place mutation and an explicit loop. Due
to the use of mutation, it must be careful to copy data that will be
used again later.

.. literalinclude:: examples/salsa20/salsa20.c
  :language: C
  :start-after: // BEGIN SALSA20
  :end-before: // END SALSA20

Note again the pervasive use of in-place mutation - as with
``s20_quarterround``, the connection between this and the functionally pure
Cryptol specification will be made clear through the SAW specification.

Salsa20 supports two key sizes: 16 and 32 bytes. Rather than writing
two separate implementations, ``Salsa20_expansion`` uses two unique
feature of Cryptol's type system to implement both at once. These
features are numbers in types and arithmetic predicates. Numbers in
types, seen earlier, are used for the lengths of sequences, and it is
possible to write functions that work on *any* length. The beginning
of the type signature reads ``{a} (a >= 1, 2 >= a) => ...``, which
says that ``a`` can only be 1 or 2. Later on in the type,
``[16*a][8]`` is used for the key length, resulting in a length of
either 16 or 32 8-bit bytes. The back-tick operator allows a program
to inspect the value of a length from a type, which is used in the
``if`` expression to select the appropriate input to
``Salsa20``. Cryptol strings, like C string literals, represent
sequences of ASCII bytes values. Their values come from the
specification.

.. literalinclude:: examples/salsa20/Salsa20.cry
  :language: Cryptol
  :start-after: // BEGIN SALSA20_EXPANSION
  :end-before: // END SALSA20_EXPANSION

The encryption function takes a tuple of three parameters: a key
``k``, an eight-byte `nonce`_ ``v``, and a message ``m`` of at most
:math:`2^{70}` bytes. In accordance with Section 10 of the
specification, it computes the `Salsa20_expansion` of the nonce and
sufficient subsequent numbers, and ``take`` truncates it to the
desired length. The message is combined with this sequence, yielding the
result.

.. _nonce: https://en.wikipedia.org/wiki/Cryptographic_nonce

.. literalinclude:: examples/salsa20/Salsa20.cry
  :language: Cryptol
  :start-after: // BEGIN SALSA20_ENCRYPT
  :end-before: // END SALSA20_ENCRYPT


SAW Specification and Verification
----------------------------------

The SAW specification for this Salsa20 implementation is comprised of
a couple of convenient helper functions, a specification for each of
the interesting functions in the Salsa20 specification (i.e. the
functions detailed in Bernstein's specification document), and a
defined command ``main`` that performs the actual verification.

One big difference between the Cryptol specification and the C
implementation is that Cryptol, a functional language, returns new
values, while C, an imperative language, tends to write new values to
a pointer's target. Typically, the C version of the program will
overwrite an argument with the value that the Cryptol version
returns. This pattern is abstracted over in ``oneptr_update_func``, a
SAWScript command that describes this relationship between C and
Cryptol versions of a function. The arguments are ``n : String`` that
names the parameter for pretty-printing, ``ty : LLVMType`` that
describes the parameter type, and the function ``f : Term`` to apply
to the parameter.

.. literalinclude:: examples/salsa20/salsa20_compositional.saw
  :language: SAWScript
  :start-after: // BEGIN ONEPTR_UPDATE
  :end-before: // END ONEPTR_UPDATE

All of Salsa20 depends on ``s20_quarterround``. Here is its
specification:

.. literalinclude:: examples/salsa20/salsa20_compositional.saw
  :language: Cryptol
  :start-after: // BEGIN QUARTERROUND
  :end-before: // END QUARTERROUND

The helper ``pointer_to_fresh`` is the same as the one in
:ref:`swap-example`. It allocates space for a new symbolic variable of
the given type, returning both the symbolic value and the pointer to
it. The symbolic values are passed to the Cryptol function
``quarterround`` to compute the expected result values. Because the
function inputs are symbolic, the outputs are also mathematical
expressions that reflect the function's behavior. These expected result
values are then used as the expected targets of the pointers in the
post-condition of the SAW specification.

The specification for ``s20_hash`` is an example of one for which
``oneptr_update_func`` is useful.

.. literalinclude:: examples/salsa20/salsa20_compositional.saw
  :language: SAWScript
  :start-after: // BEGIN SALSA20
  :end-before: // END SALSA20

.. _compositional-verification:

Putting everything together, ``main`` verifies the implementation
functions according to these specifications. ``main`` has the type
``TopLevel ()`` --- this is the type of commands that can be run at
the top level of a SAWScript program. In :ref:`swap-example`,
``crucible_llvm_verify`` was used on its own, and its return value was
discarded. However, verification actually returns a useful result: it
returns an association between a specification and the fact that the
given function has been verified to fulfill it. In SAWScript, this
association has the type ``CrucibleMethodSpec``. Because
``crucible_llvm_verify`` is a command, the returned value is saved
using the ``<-`` operator.

The third argument to ``crucible_llvm_verify`` is a list of
``CrucibleMethodSpec`` objects. While performing verification, the
work that was done to construct a ``CrucibleMethodSpec`` is
re-used. Specifically, instead of recursively symbolically executing a
verified function, the prior specification is used as an
axiomatization of its behavior. In the definition of ``main``, the
highlighted lines pass the results of earlier verifications along:

.. literalinclude:: examples/salsa20/salsa20_compositional.saw
  :language: Cryptol
  :start-after: // BEGIN MAIN
  :end-before: // END MAIN
  :emphasize-lines: 4-11

This example also uses the fourth argument to
``crucible_llvm_verify``. During symbolic execution, conditionals
require that both branches be explored. If the fourth argument is
``true``, then an SMT solver is used to rule out impossible
branches. For some problems, the overhead of the solver exceeds the
time saved on exploring branches; for others, a short time spent in
the solver saves a long time spent in the symbolic execution
engine. Ruling out impossible branches can also allow termination of
programs in which the number of iterations can depend on a symbolic
value. This is called path satisfiability checking.

The 16-byte version of Salsa20 is not verified, because the C program
does not implement it. Also, Salsa20 is verified only with respect to
some particular message lengths, because SAW is not yet capable of
verifying infinite programs. This is why ``main`` verifies multiple
lengths, in the hope that this is sufficient to increase our
confidence.


Comparing Compositional and Non-compositional Verification
----------------------------------------------------------

In ``examples/salsa20``, there are two SAW specifications:
``salsa20_compositional.saw``, which contains ``main`` as presented above, and
``salsa20_noncompositional``, which replaces the ``CrucibleMethodSpec`` list
parameter in each call to ``crucible_llvm_verify`` with the empty list,
effectively disabling compositional verification. The one exception to this is
in the verification of ``s20_hash``; not using compositional verification for
this function did not terminate in a reasonable amount of time.

These two verification tasks were run on a 2019 15-inch MacBook Pro, 2.4 GHz
8-Core Intel i9 processor, 32 GB DDR4 RAM. The values shown are the average
over five runs:

+------------------+---------------+-------------------+
|                  | Compositional | Non-Compositional |
+==================+===============+===================+
| Average Time (s) |      2.64     |       5.39        |
+------------------+---------------+-------------------+

Even with this limited data set, the benefits of using compositional
verification are clear: There's effectively a 2x increase in speed in this
example, even accounting for the fact that the verification of ``s20_hash``
is still treated compositionally.

Exercise: Rot13
~~~~~~~~~~~~~~~

Rot13 is a Caesar cipher that is its own inverse. In it, each letter
is mapped to the letter that is 13 places greater than it in the
alphabet, modulo 26. Non-letters are untouched, and case is
preserved. For instance, "abc" becomes "nop", and "SAW is fun!"
becomes "FNJ vf sha!".

Your task is to implement rot13 in C, and verify it using SAW.

Start by writing a function that performs a single character of rot13,
assuming 7-bit ASCII encoding. Verify it using SAW and Cryptol.

Then, write a function that uses your single-character rot13 to
perform rot13 on a string with precisely 20 characters in it. Verify
this using SAW and Cryptol with compositional verification.



.. rubric:: Footnotes

.. [#salsa20wiki] https://en.wikipedia.org/wiki/Salsa20
