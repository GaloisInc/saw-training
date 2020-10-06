Getting Started
===============

This tutorial is intended to be an interactive experience. It is much
easier to learn how to use a new tool by actually using it, making
mistakes, and recovering from them. Please follow along in the
provided exercises.

Background
----------

This tutorial is written for programmers who know C, but who do not
necessarily have any experience with formal verification. Deep
knowledge of C is not required, but familiarity with pointers and the
concept of undefined behavior are assumed. It is not necessary to be
able to identify undefined behavior on sight.


Notation
--------

Code examples, filenames, and commands that should be literally typed
into a computer are represented with ``monospace font``. For instance,
the file ``main.c`` might contain the function

.. code-block:: C

  int main(int argc, char** argv) { ... }

which has an argument called ``argc``. At times, :math:`\mathit{italic\ text}`
is used to represent mathematical variables. For instance, when relating programs
to mathematical specifications, the program variable ``n`` might have the
mathematical value :math:`x^2`.

Exercises: Initial setup
------------------------

The first step is to install all of the necessary tools. For this
tutorial, you'll need the following:

  SAW
    SAW can be dowloaded from `the SAW web page <https://saw.galois.com/downloads.html>`_.

  Yices and Z3
    This tutorial uses
    `Yices <https://yices.csl.sri.com/>`_ and
    `Z3 <https://github.com/Z3Prover/z3/releases>`_. If you plan to
    work seriously with SAW, it is also a good idea to install the
    other solvers listed on the `SAW download page
    <https://saw.galois.com/downloads.html>`_.

  Cryptol
    Cryptol is included with SAW. Please use the version of Cryptol
    that's included, because each SAW release requires a specific
    Cryptol version.

  LLVM and Clang
    Please make sure that you have LLVM and clang installed.

To make sure that you have everything working, download the
:download:`example files </examples.tar.gz>`. In the ``examples/swap``
directory, run the following commands::

    make swap.bc
    saw swap_cryptol.saw
    cryptol Swap.cry

If everything succeeds, you'll be at a Cryptol prompt. Use ``:q`` to
exit Cryptol.
