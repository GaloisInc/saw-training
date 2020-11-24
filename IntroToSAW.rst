.. _pop-example:

First Example: Counting Set Bits
================================

Most developers are used to techniques like testing, continuous integration, and thoughtful documentation that can help prevent mistakes from being introduced into a system during its development and evolution. These techniques can be relatively effective, but they risk missing certain classes of bugs. For the most important systems, like those that protect human life or information security, it can make sense to also use formal *verification*, in which a program is mathematically proved to be correct for *all* inputs.

Testing takes the *actual binary executable* and runs it on a *subset* of the possible inputs to check for expected outputs. The downside of this approach is that tests may miss some critical case. Compared to testing, verification is the process of building a *mathematical model of the software* and *proving properties* of that model for *all* possible inputs. When using verification, the primary concern is that the model is inaccurate. Fortunately these techniques complement each other: testing can help validate that a model is accurate and verification can help build confidence that the tests aren't missing uncommon inputs that trigger misbehavior. In combination, testing and verification can build confidence in the *trustworthiness* of a program.

.. DTC - my worry is not that the model is incorrect, it's that the spec is wrong. 

In this lesson you'll learn how to use a system called SAW, the Software Analysis Workbench, to build models of functions written in C. You'll learn how to specify what those functions are *supposed* to do, and how to write a program in :term:`SAWScript`, the SAW configuration language, that orchestrates the proof that the functions meet their specifications for all possible inputs.


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

Write a version of ``pop_count`` that you believe to be correct, and also a version that includes the kind of error that could be made by accident (perhaps by adding a typo to the optimized version).  Add them as ``pop_count_ok`` and ``pop_count_broken1`` to ``popcount.c`` in the examples directory.

Testing Programs
----------------

You're not likely to be able to convince yourself that the optimized ``pop_count`` function is correct just by inspection. A unit test, like the following ``pop_check`` can help:

.. literalinclude:: examples/intro/popcount.c
  :language: C
  :start-after: // BEGIN POP_CHECK
  :end-before: // END POP_CHECK

There are some downsides to testing only with chosen values, however. First off, these tests are usually selected by the author of the code, and there is a risk that important values are not tested. This can be ameliorated by a systematic, disciplined approach to choosing test values, but it can never be completely eliminated. This particular unit test is likely to catch completely buggy versions of ``swap``, but not subtle or tricky ones.

A second approach to testing is to choose many random values at each execution, as in ``random_value_test``. This approach may eventually find subtle or tricky mistakes, but not reliably or in a reasonable amount of time.

.. literalinclude:: examples/intro/popcount.c
  :language: C
  :start-after: // BEGIN POP_RANDOM_VALUE_TEST
  :end-before: // END POP_RANDOM_VALUE_TEST

Finally, one could attempt to exhaustively check the values by enumerating and testing *all possible* combinations. However, even in the simple case of ``pop_count``, which only takes one 32-bit integer, this will take many hours. With a 64-bit version of the program the test would take longer than a normal human lifetime, so this technique is not practical for ongoing software development.

The way formal verification addresses this is by reasoning about *mathematical models* of a program, which allows it to eliminate huge regions of the state space with single steps. There are many tools and techniques for performing full formal verification, each suitable to different classes of problem. SAW is particularly suited to imperative programs that don't contain potentially-unbounded loops. In general, the cost of verification is that it requires specialized knowledge and developing mathematical proofs can take much longer than writing test cases. However, for many programs, automated tools like SAW can be used with similar levels of effort to testing, but resulting in much stronger guarantees. At the same time, re-checking a proof can sometimes be *much faster* than testing large parts of the input space, leading to quicker feedback during development.

Exercise: Testing ``popcount``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Write a test that detects the defects in your ``pop_count_broken1`` function, and try to verify that your ``pop_count_ok`` and the optimized ``pop_count`` function have no defects by using manual and random testing. How much confidence do those techniques provide? Finally, if you're feeling adversarial, write a ``pop_count_broken2`` that is only incorrect for exactly one input value. How likely is a randomized test going to detect the one input value?

Symbolic Execution
------------------

The way SAW can prove properties about programs is by converting them into an internal representation that is much closer to a pure mathematical function. For instance, ``pop_count`` might be converted to a function like:

.. math::

    pop\_count(bit\_string) = \sum_{i=0}^{32}{bit\_string_i}

In which the details of the call stack, registers vs. memory and the specific execution model of the CPU have been removed. The technique for doing this conversion is called *symbolic execution* or *symbolic simulation*. It works by first replacing some of the inputs to a program with *symbolic values*, which are akin to mathematical variables. The term *concrete values* is used to describe honest-to-goodness bits and bytes. As the program runs, operations on symbolic values result in descriptions of operations rather than actual values. Just as adding ``1`` to the concrete value ``5`` yields the concrete value ``6``, adding ``1`` to the symbolic value :math:`x` yields the symbolic value :math:`x+1`. Incrementing the values again yields ``7`` and :math:`x+2`, respectively. By simulating the entire function this way, SAW creates a mathematical function out of the C function you provide.

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

SAW is a tool for extracting models from compiled programs and then applying both automatic and manual reasoning to compare them against a :term:`specification` of some kind. SAW builds models of programs by *symbolically executing* them, and is capable of building models from LLVM bitcode, JVM bytecode, x86 machine code, Rust's MIR internal representation, and a number of other formats.

The first step to using SAW on ``pop_count`` is to use ``clang`` to construct its representation in LLVM bitcode. It is important to pass ``clang`` the ``-O1`` flag, because important symbols are stripped at higher optimization levels, while lower optimization levels yield code that is less amenable to symbolic execution. It can be convenient to include this rule in a ``Makefile``:

.. literalinclude:: examples/intro/Makefile
  :language: make
  :start-after: # Build commands for bitcode
  :end-before: # End build commands for bitcode

.. index:: SAWScript

After building the LLVM bitcode file (by typing ``make popcount.bc``), the next step is to use SAW to verify that the program meets its :term:`specification`. SAW is controlled using a language called :term:`SAWScript`. SAWScript contains commands for loading code artifacts, for describing program specifications, for comparing code artifacts to specifications, and for helping SAW in situations when fully automatic proofs are impossible.

The SAWScript to verify ``pop_count`` is:

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
2. Lines 4--8 defines the ``pop_is_ok`` SAWScript function, which sets up the symbolic inputs to the ``pop_spec`` function, calls the function on those symbolic inputs and asserts that the return value is True.
3. Line 10 instructs SAW to verify that ``pop_is_ok`` is true for *all possible* input values.

The LLVM module is loaded using the ``llvm_load_module`` command. This command takes a string that contains the filename as an argument, and returns the module itself. In SAWScript, the results of a command are saved using the ``<-`` operator; here, the name ``popmod`` is made to refer to the module.

.. index:: precondition

Here, the precondition consists of creating one symbolic variable. Internally, symbolic variables are represented in the internal language :term:`SAWCore`. ``symbolic_variable`` takes two arguments: its type and the name for the symbolic variable, a string, which can show up in error messages. After the precondition, the :term:`SAWScript` variable ``x`` is bound to the respective symbolic value :math:`x`. In more complicated verifications the preconditions are more interesting, as we'll see soon.

The function is invoked using the ``execute`` command, which takes an array of SAWScript variables that correspond to the function's arguments. The function being executed is the one named by the string argument in the call to ``llvm_verify``.

.. index:: postcondition

In the postcondition the expected return value of the function is specified using ``returns``. In this example, the function is expected to return ``TRUE``.

Translated to English, ``pop_is_ok`` says:

    Let :math:`x` be a 32-bit integer. The result of calling ``pop_spec_check`` on it is ``TRUE``.

If verification reports success, we know that this is the case *for all possible values of* :math:`x` *and* :math:`y`.

In other words, ``pop_is_ok`` wraps the C function ``pop_spec_check``. This C function computes the believed-correct result (by calling ``pop_spec``), calls the ``pop_count`` function we are analyzing and returns TRUE if the results agree. The SAW wrapper creates the symbolic input variable, executes the function on its input, and ensures that the return value is ``TRUE``.

.. note::

    :term:`SAWScript` distinguishes between defining a name and saving the result of a command. Use ``let`` to define a name, which may refer to a command or a value, and ``<-`` to run a command and save the result under the given name. Comparing to C, ``let`` is closer to ``#define`` than any other construct - it's just creating a new name for another expression. Nothing happens until that new name is later used in an expression that is being evaluated.

Finally, on line 11, the ``llvm_verify`` command is used to instruct SAW to carry out verification. The  arguments are ``swapmod``, which specifies the LLVM module that contains the code to be verified; ``"pop_spec_check"``, the function to be symbolically executed; ``pop_is_ok``, and the SAW specification to check ``"pop_spec_check"`` against. The empty list (``[]``) is an optional list of previously proven statements, which is used in larger verification projects as described :ref:`later in this tutorial<compositional-verification>`. This verification script provides the same level of assurance that exhaustive testing would provide, but it completes in a tiny fraction of the time, fast enough to be part of a standard CI workflow.

Exercise: Verifying Your ``pop_count`` implementations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

As a first exposure to formal verification, a successful verification of a provided example isn't quite as compelling as one that verifies or finds a bug in code we wrote, so let's do that for your ``pop_count_ok`` and ``pop_count_broken1`` from the first exercise. Edit your ``popcount.c`` file to add a wrapper for each of them, like ``pop_spec_check``, to compare their results to that of calling ``pop_check``. Then in your ``pop.saw`` file add additional calls to ``llvm_verify`` passing in your wrapper functions as the 2nd string argument (in place of ``"pop_spec_check"``). Finally, verify (or find errors in) them by re-creating the bitcode and re-running saw:

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

As in the output above, you should see one successful verification (for the wrapper corresponding to ``pop_count_ok``) and one failed one (for ``pop_count_broken1``). SAW's messages for failed verifications are quite verbose, and we'll learn more about what they're saying later, but the most important part is the counterexample, which is a concrete input value that fails to return ``TRUE``. If you created the bonus ``popcount_broken2`` in the exercise above, which is only incorrect for exactly one input value, it's pretty cool to see SAW come up with exactly that counterexample without any guidance from you.

.. _pop-cryptol:

Cryptol
-------

:term:`SAWScript` has facilities for describing variables, memory layouts (more on that in the next section) and pre- and postconditions, but not for specifying algorithms. It is often used together with Cryptol, a domain-specific language for implementing low-level cryptographic algorithms or DSP transforms that reads much like a mathematical description. This helps bridge the gap between formal descriptions and real implementations.

A Cryptol specification for ``pop_count`` looks like this:

.. literalinclude:: examples/intro/Popcount.cry
  :language: Cryptol
  :start-after: // Begin Cryptol Popcount
  :end-before: // End Cryptol Popcount

This Cryptol code creates a list of partial population counts by iterating through the input bits, adding one when it finds a ``1`` bit. It returns the final element of this list with ``! 0``. If you would like to read more about how to understand and write your own Cryptol specifications, the `Cryptol documentation, <https://cryptol.net/documentation.html>`_ which includes the book `Programming Cryptol, <https://cryptol.net/files/ProgrammingCryptol.pdf>`_ is a good place to start.

Here is the SAWScript code that verifies ``pop_count`` is equivalent to the Cryptol specification:

.. literalinclude:: examples/intro/popCryptol.saw
  :language: SAWScript
  :start-after: // Begin Cryptol popcount SAW
  :end-before: // End Cryptol popcount SAW
  :linenos:

The connection to the Cryptol specification is on line 9 inside the ``pop_cryptol_check`` function, which calls the Cryptol version of ``popCount`` by surrounding it with double curly braces. Any expression surrounded by ``{{ ... }}`` is interpreted as Cryptol, and converted to SAW Terms.

One of the benefits of having the specification written in Cryptol is that you don't need to write C wrapper functions, like ``pop_check``, but instead can verify that the return value of the C function is equal to the Cryptol specification, which is a nicer separation of implementation and verification. It allows us to test additional implementations of ``pop_count`` simply by adding a line in the SAWScript, as below:

.. literalinclude:: examples/intro/popCryptol.saw
  :language: SAWScript
  :start-after: // Begin Cryptol additional verifications
  :end-before: // End Cryptol additional verifications

.. note::

  The SAWScript function ``pop_cryptol_check`` creates its symbolic variable slightly differently to the original example: it assigns the pair ``(xs,xt)`` to the result of calling ``symbolic_setup_tuple``. This is an artifact of some details we'll explore in the next section. The short explanation is that SAW's memory model is more expressive than a mathematical system like Cryptol's: it can reason about symbolic *pointers* in addition to variables. The SAWScript type that interoperates with Cryptol is a :term:`Term`, and is a subclass of the SAWScript type called a :term:`SetupValue` (which can also be a pointer). SAWScript does not yet automatically treat Terms as SetupValues, so in many SAWScript programs we have to keep track of both versions of the same symbolic variable and pass them, as appropriate, to SAWScript functions and Cryptol functions. In this case, on line 8, the ``execute`` needs a *SetupValue*, which is the first element of the tuple returned by ``symbolic_setup_tuple``, and the ``returns``, which calls out to Cryptol with the double-curly brace syntax ``{{ popCount xt }}`` requires the *Term* element of the tuple.

.. _pop-code-evolution:

Evolving Specifications and Code Together
-----------------------------------------

As needs change, programs and their specifications must evolve. A verification-oriented workflow can help maintain a correspondence between updated specifcations and code.

This section describes the process of adapting ``pop_count`` to changes in a specification. Please follow along yourself to gain experience with understanding the feedback that SAW provides during this process.

Our goal is to implement and verify a 64-bit population count function. The Cryptol code to do this is a straightforward extension of our earlier specification:

.. literalinclude:: examples/intro/Popcount.cry
  :language: Cryptol
  :start-after: // Begin Cryptol Popcount64
  :end-before: // End Cryptol Popcount64

In fact, you'll note that the specification iself is the same - all that changed is the declaration that its input and output types have 64-bits instead of 32.

Exercise: Verifying A 64-bit Population Count
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Above, we have a Cryptol specification for 64-bit population count. In this exercise, implement a C function with this signature: ``uint64_t pop_count_64 (uint64_t x)``. Add the Cryptol specification to your ``Popcount.cry`` file, and add the verification to your ``pop.saw`` file. You might want to comment-out the attempted verification of ``popcount_broken1`` to make sure ``saw`` completes the verification (and reduce the noise).

One approach you could take is to shift / mask the input word appropriately and return the sum of two calls to the 32-bit version.


..  LocalWords:  cryptographic
