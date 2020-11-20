Glossary
--------

.. glossary::

  compositional verification
    A verification technique based on the idea that, when proving properties of a given method or function, we can make use of properties we have already proved about its callees.

  Cryptol
    A specification language for algorithms. Used as the notation for :term:`SAWCore` in :term:`SAWScript`.

  proof maintenance
    The process of keeping verification artifacts, such as specifications and proofs, up to date with changes in a software system over time.

  SAWCore
    The internal representation for programs in SAW.

  SAWScript
    The language used to write specifications and describe
    verification tasks in SAW.

  SetupValue
    A SAWScript SetupValue can be either a :term:`Term` or a pointer.
    Arguments passed to symbolically executed functions must be
    SetupValues.

  specification
    A description of what is desired of a program. Specifications can
    be written in anything from informal English to precise,
    machine-readable logical formulations.

  symbolic execution
    The process of running a program where some input values are mathematical expressions (also called a :term:`symbolic value`) instead of actual values. If the program terminates, the result is a mathematical expression that characterizes its behavior.

  symbolic value
    A program value that is a mathematical expression, like :math:`\left|x + \left\lfloor y \right\rfloor \right|`, instead of concrete bits in memory.

  testing
    The practice of finding empirical evidence that a program lives up
    to a :term:`specification`.

  Term
    A SAWScript Term is a symbolic value that can only represent
    values, not pointers. This is in contrast to a :term:`SetupValue`,
    which is a superclass of Terms and Pointers.
    Arguments passed to Cryptol functions must be Terms.

  verification
    The practice of finding mathematical evidence that a program lives
    up to a :term:`specification`.
