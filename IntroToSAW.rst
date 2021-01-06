.. _pop-example:

First Example: Counting Set Bits
================================

Most developers are used to techniques like testing, continuous integration, and thoughtful documentation that can help prevent mistakes from being introduced into a system during its development and evolution. These techniques can be relatively effective, but they risk missing certain classes of bugs. For the most important systems, like those that protect human life or information security, it can make sense to also use formal *verification*, in which a program is mathematically proved to be correct for *all* inputs.

Testing takes the *actual binary executable* and runs it on a *subset* of the possible inputs to check for expected outputs. The downside of this approach is that tests may miss some critical case. Compared to testing, verification is the process of building a *mathematical model of the software* and *proving properties* of that model for *all* possible inputs. 


In this lesson you'll learn how to use a system called SAW, the Software Analysis Workbench, to build models of functions written in C. You'll learn how to specify what those functions are *supposed* to do, and how to write a program in :term:`SAWScript` that orchestrates the proof that the functions meet their specifications.


The Code
--------

The first program to be verified is ``pop_count``. This function takes a 32-bit integer and returns the number of bits that are set ("populated"). For example ``pop_count(0)`` is 0, ``pop_count(3)`` is 2, and ``pop_count(8)`` is 1. This description is an English language *specification* of the ``pop_count`` function. A :term:`specification` can be written in a number of formats, including English sentences, but also in machine-readable forms. The advantage of machine-readable specifications is that they can be used as part of an automated workflow.

.. note::

    The ``pop_count`` function has uses in many kinds of algorithms and has an `interesting <https://vaibhavsagar.com/blog/2019/09/08/popcount/>`_ `folklore <https://groups.google.com/g/comp.arch/c/UXEi7G6WHuU/m/Z2z7fC7Xhr8J>`_.

Here is a sophisticated implementation of ``pop_count`` from the book *Hacker's Delight* by Henry S. Warren Jr.:

.. literalinclude:: examples/intro/popcount.c
  :language: C
  :start-after: // BEGIN POP_COUNT
  :end-before: // END POP_COUNT

Exercise: A Safe and a Broken ``pop_count``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Write a version of ``pop_count`` that you believe to be correct, and also a version that includes the kind of error that could be made by accident (perhaps by adding a typo to the optimized version).  Add them as ``pop_count_ok`` and ``pop_count_broken1`` to ``popcount.c`` in the `examples/intro` directory.

Testing Programs
----------------

You're not likely to be able to convince yourself that the optimized ``pop_count`` function is correct just by inspection. A unit test, like the following ``pop_check`` can help:

.. literalinclude:: examples/intro/popcount.c
  :language: C
  :start-after: // BEGIN POP_CHECK
  :end-before: // END POP_CHECK

There are some downsides to testing only with chosen values, however. First off, these tests are usually selected by the author of the code, and there is a risk that important values are not tested. This can be ameliorated by a systematic, disciplined approach to choosing test values, but it can never be completely eliminated. This particular unit test is likely to catch egregiously buggy versions of ``popcount``, but not subtle or tricky bugs.

A second approach to testing is to choose many random values at each execution. This approach may eventually find subtle or tricky mistakes, but not reliably or in a predictable amount of time.

Testing with random values requires an executable specification. This specification may just describe some properties of the output (e.g. that the length of two appended lists is the sum of the lengths of the input lists, or that the output of a sorting function is sorted), or it may be a simpler, more straightforward version of the code that uses an easier algorithm. An executable specification for ``popcount`` can loop over the bits in the word, masking them off one at a time. While this implementation is straightforward, it is also slow.

.. literalinclude:: examples/intro/popcount.c
  :language: C
  :start-after: // BEGIN POP_SPEC
  :end-before: // END POP_SPEC

The function ``random_value_test`` performs randomized testing of a provided population count function, comparing its output to that of ``pop_spec``. When they are not identical, it prints the offending input, which can aid in debugging.

.. literalinclude:: examples/intro/popcount.c
  :language: C
  :start-after: // BEGIN POP_RANDOM_VALUE_TEST
  :end-before: // END POP_RANDOM_VALUE_TEST

Finally, one could attempt to exhaustively check the values by enumerating and testing *all possible* combinations. In the simple case of ``pop_count``, which only takes one 32-bit integer, this will take about 20 seconds. With a 64-bit version of the program, however, the test would take longer than a normal human lifetime, so this technique is not practical for ongoing software development.

The way formal verification addresses this is by reasoning about mathematical models of a program, which allows it to eliminate huge regions of the state space with single steps. There are many tools and techniques for performing full formal verification, each suitable to different classes of problem. SAW is particularly suited to imperative programs that don't contain potentially-unbounded loops. In general, the cost of verification is that it requires specialized knowledge and developing mathematical proofs can take much longer than writing test cases. However, for many programs, automated tools like SAW can be used with similar levels of effort to testing, but resulting in much stronger guarantees. At the same time, re-checking a proof can sometimes be *much faster* than testing large parts of the input space, leading to quicker feedback during development.

Exercise: Testing ``popcount``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Write a test that detects the defects in your ``pop_count_broken1`` function, and also check that your ``pop_count_ok`` and the optimized ``pop_count`` function have no defects by using manual and random testing. How much confidence do those techniques provide?

Finally, consider ``pop_count_broken2``, which is only incorrect for exactly one input value. Check how often the randomized test detects the one buggy input.

.. literalinclude:: examples/intro/popcount.c
  :language: C
  :start-after: // BEGIN POP_BROKEN2
  :end-before: // END POP_BROKEN2


Symbolic Execution
------------------

The way SAW can prove properties about programs is by converting them into an internal representation that is much closer to a pure mathematical function. For instance, ``pop_count`` might be converted to a function like:

.. math::

    pop\_count(bit\_string) = \sum_{i=0}^{32}{bit\_string_i}

In this version, the details of the call stack, registers vs. memory and the specific execution model of the CPU have been removed. The technique for doing this conversion is called *symbolic execution* or *symbolic simulation*. It works by first replacing some of the inputs to a program with *symbolic values*, which are akin to mathematical variables. The term *concrete values* is used to describe honest-to-goodness bits and bytes. As the program runs, operations on symbolic values result in descriptions of operations rather than actual values. Just as adding ``1`` to the concrete value ``5`` yields the concrete value ``6``, adding ``1`` to the symbolic value :math:`x` yields the symbolic value :math:`x+1`. Incrementing the values again yields ``7`` and :math:`x+2`, respectively. By simulating the entire function this way, SAW creates a mathematical function out of the C function you provide.

There are complications, of course, such as what to do with conditional branches, but as a user of the tool you won't have to worry about them except when they introduce limitations to what you can reason about. The main such limitation is that symbolic simulation can't effectively deal with loops whose termination depends on a symbolic value. For example, this simple implementation of ``add`` would not be easily analyzed:

.. code-block:: C

    unsigned int add(unsigned int x, unsigned int y) {
        for (unsigned int i = 0; i < y; i ++) {
            x++;
        }
        return x;
    }

The problem is that the loop termination depends on the symbolic value :math:`y`, rather than on some pre-determined concrete number. This means that each time through the ``for`` loop two new branches must be explored: one in which the present concrete value of ``i`` is less than the symbolic value of :math:`y`, and one in which it is not. The key thing to remember is that symbolic execution is most applicable to programs that "obviously" terminate, or programs in which the number of loop iterations do not depend on which specific input is provided.

Running SAW
-----------

.. note::
  This section uses a library of SAW helpers, in the file ``helpers.saw``. If you're comparing this text to the SAW manual, you may notice that a few operations have been abbreviated.

SAW is a tool for extracting models from compiled programs and then applying both automatic and manual reasoning to compare them against a :term:`specification` of some kind. SAW builds models of programs by *symbolically executing* them, and is capable of building models from LLVM bitcode, JVM bytecode, x86 machine code, Rust's MIR internal representation, and a number of other formats.

The first step to verifying ``pop_count`` with SAW is to use ``clang`` to construct its representation in LLVM bitcode. It is important to pass ``clang`` the ``-O1`` flag, because important symbols are stripped at higher optimization levels, while lower optimization levels yield code that is less amenable to symbolic execution. The `-g` flag leaves symbols in the output which helps SAW produce helpful messages when verification fails. It can be convenient to include this rule in a ``Makefile``:

.. literalinclude:: examples/intro/Makefile
  :language: make
  :start-after: # Build commands for bitcode
  :end-before: # End build commands for bitcode

.. index:: SAWScript

After building the LLVM bitcode file (by typing ``make popcount.bc``), the next step is to use SAW to verify that the program meets its :term:`specification`. SAW is controlled using a language called :term:`SAWScript`. SAWScript contains commands for loading code artifacts, for describing program specifications, for comparing code artifacts to specifications, and for helping SAW in situations when fully automatic proofs are impossible.

The specific fact to be verified using SAW is that ``pop_count`` and ``pop_spec`` always return the same answer, no matter their input. For any particular input, this can be checked using ``pop_spec_check``:

.. literalinclude:: examples/intro/popcount.c
  :language: C
  :start-after: // BEGIN POP_SPEC_CHECK
  :end-before: // END POP_SPEC_CHECK


The SAWScript to verify ``pop_count`` is really checking that ``pop_spec_check`` always returns true.

.. literalinclude:: examples/intro/pop.saw
  :language: SAWScript
  :linenos:

To execute the verification, we invoke ``saw`` on the SAWScript file:

.. code-block::

  $ saw pop.saw
  [20:24:45.159] Loading file "/.../pop.saw"
  [20:24:45.160] Loading file "/.../helpers.saw"
  [20:24:45.282] Verifying pop_spec ...
  [20:24:45.282] Simulating pop_spec ...
  [20:24:45.291] Checking proof obligations pop_spec ...
  [20:24:46.212] Proof succeeded! pop_spec

The ``Proof succeeded!`` message indicates to us that our ``pop_spec_check`` function returns True for all possible inputs. Hooray!

Returning to the SAWScript we used, it has three parts:

1. Lines 1--2 load helper functions and the LLVM module to be verified. This step builds the model from your code.
2. Lines 4--8 defines the ``pop_is_ok`` SAWScript specification, which sets up the symbolic inputs to the ``pop_spec`` function, calls the function on those symbolic inputs, and asserts that the return value is True.
3. Line 10 instructs SAW to verify that ``pop_is_ok`` is true for *all possible* input values.

The LLVM module is loaded using the ``llvm_load_module`` command. This command takes a string that contains the filename as an argument, and returns the module itself. In SAWScript, the results of a command are saved using the ``<-`` operator; here, the name ``popmod`` is made to refer to the module.

.. index:: precondition

SAW specifications have three main parts:

1. Preconditions which state what the code being verified may assume to be true when it is called,
2. Instructions for executing the code.
3. Postconditions which state what the code must ensure to be true after it is called.


Here, the precondition consists of creating one symbolic variable. Internally, symbolic variables are represented in the internal language :term:`SAWCore`. ``symbolic_variable`` takes two arguments: the new variable's type and a string that names the symbolic variable (which may show up in error messages). After the precondition, the :term:`SAWScript` variable ``x`` is bound to the respective symbolic value :math:`x`. In more complicated verifications the preconditions are more interesting, as we'll see soon.

The function is invoked using the ``execute`` command, which takes an array of SAWScript variables that correspond to the function's arguments. The function being executed is the one named by the string argument in the call to ``llvm_verify``.

.. index:: postcondition

In the postcondition, the expected return value of the function is specified using ``returns``. In this example, the function is expected to return ``TRUE``.

Translated to English, ``pop_is_ok`` says:

    Let :math:`x` be a 32-bit integer. The result of calling ``pop_spec_check`` on :math:`x` is ``TRUE``.

If verification reports success, we know that this is the case *for all possible values of* :math:`x` *and* :math:`y`.

In other words, ``pop_is_ok`` wraps the C function ``pop_spec_check``. This C function computes the believed-correct result (by calling ``pop_spec``), calls the ``pop_count`` function we are analyzing and returns TRUE if the results agree. The SAW wrapper creates the symbolic input variable, executes the function on its input, and ensures that the return value is ``TRUE``.

.. note::

    :term:`SAWScript` distinguishes between defining a name and saving the result of a command. Use ``let`` to define a name, which may refer to a command or a value, and ``<-`` to run a command and save the result under the given name. Defining a command with ``let`` is analogous to defining a C function, and invoking commands with ``<-`` is analogous to calling it.

.. Comparing to C, ``let`` is closer to ``#define`` than any other construct - it's just creating a new name for another expression. Nothing happens until that new name is later used in an expression that is being evaluated.

The  arguments to ``llvm_verify`` (on line 10 above) are ``popmod``, which specifies the LLVM module that contains the code to be verified; ``"pop_spec_check"``, the C function to be symbolically executed; and ``pop_is_ok``, the SAW specification to check ``"pop_spec_check"`` against. The empty list (``[]``) is an optional list of previously proven statements, which is used in larger verification projects as described :ref:`later in this tutorial<compositional-verification>`. This verification script provides the same level of assurance that exhaustive testing would provide, but it completes in a tiny fraction of the time, fast enough to be part of a standard CI (continuous integration) workflow.

Exercise: Verifying Clever Versions of ``popcount``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following versions of ``popcount`` are quite different from the preceding implementations, but they should always return the same value. For both ``pop_count_mul`` and ``pop_count_sparse``, do the following:

1. Write a C function, analogous to ``pop_spec_check``, that relates ``pop_spec`` to the new implementation.

2. Use ``pop_is_ok`` in ``pop.saw`` together with additional calls to ``llvm_verify`` to asserts that the modified versions ``pop_spec_check`` also always return true. The string argument to ``llvm_verify`` states the name of the C function being verified - modify it to point to your new specification.

3. Use SAW to verify the implementation. Remember to rebuild the bitcode file after modifying the C sources.

.. literalinclude:: examples/intro/popcount.c
  :language: C
  :start-after: // BEGIN POP_MUL
  :end-before: // END POP_MUL

.. literalinclude:: examples/intro/popcount.c
  :language: C
  :start-after: // BEGIN POP_SPARSE
  :end-before: // END POP_SPARSE


Exercise: Verifying Your ``pop_count`` Implementations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Verification is useful for more than just carefully-chosen examples. This exercise is about your programs.

Start with your solutions ``pop_count_ok`` and ``pop_count_broken1`` from the first exercise. Repeat the tasks from the previous exercise, creating specifications and extending ``pop.saw`` to attempt to verify the functions. 

.. code-block::

  $ make popcount.bc
  $ saw pop.saw
  ...
  [19:27:38.518] Proof succeeded! pop_ok_check
  [19:27:38.520] Verifying pop_broken1_check ...
  ... many lines deleted
  [19:27:38.856] ----------Counterexample----------
  [19:27:38.856]   x: 3735928559
  [19:27:38.856] ----------------------------------

As in the output above, you should see one successful verification (for the wrapper corresponding to ``pop_count_ok``) and one failed one (for ``pop_count_broken1``). SAW's messages for failed verifications are quite verbose, but the most important part is the counterexample, which is a concrete input value for which the program fails to return ``TRUE``. If you created the bonus ``popcount_broken2`` in the exercise above, which is only incorrect for exactly one input value, SAW will come up with exactly that counterexample without any guidance from you.



..  LocalWords:  cryptographic
