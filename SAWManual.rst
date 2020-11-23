The SAW Manual
==============

Overview
--------
The Software Analysis Workbench (SAW) is a tool for constructing mathematical models of the computational behavior of software, transforming these models, and proving properties about them.

SAW can currently construct models of a subset of programs written in Cryptol, LLVM (and therefore C), and JVM (and therefore Java). The models take the form of typed functional programs, so in a sense SAW can be considered a translator from imperative programs to their functional equivalents. Various external proof tools, including a variety of SAT and SMT solvers, can be used to prove properties about the functional models. SAW can construct models from arbitrary Cryptol programs, and from C and Java programs that have fixed-size inputs and outputs and that terminate after a fixed number of iterations of any loop (or a fixed number of recursive calls). One common use case is to verify that an algorithm specification in Cryptol is equivalent to an algorithm implementation in C or Java.

The process of extracting models from programs, manipulating them, forming queries about them, and sending them to external provers is orchestrated using a special purpose language called SAWScript. SAWScript is a typed functional language with support for sequencing of imperative commands.

The rest of this document first describes how to use the SAW tool, ``saw``, and outlines the structure of the SAWScript language and its relationship to Cryptol. It then presents the SAWScript commands that transform functional models and prove properties about them. Finally, it describes the specific commands available for constructing models from imperative programs.


Invoking SAW
------------

The primary mechanism for interacting with SAW is through the ``saw`` executable included as part of the standard binary distribution. With no arguments, ``saw`` starts a read-evaluate-print loop (:term:`REPL`) that allows the user to interactively evaluate commands in the SAWScript language. With one file name argument, it executes the specified file as a SAWScript program.

In addition to a file name, the ``saw`` executable accepts several command-line options:

``-h``, ``-?``, ``--help``
    Print a help message.

``-V``, ``--version``
    Show the version of the SAWScript interpreter.

``-c path``, ``--classpath=path``
    Specify a colon-delimited list of paths to search for Java classes.

``-i path``, ``--import-path=path``
    Specify a colon-delimited list of paths to search for imports.

``-t``, ``--extra-type-checking``
    Perform extra type checking of intermediate values.

``-I``, ``--interactive``
    Run interactively (with a :term:`REPL`). This is the default if no other arguments are specified.

``-j path``, ``--jars=path``
    Specify a colon-delimited list of paths to ``.jar`` files to search for Java classes.

``-d num``, ``--sim-verbose=num``
    Set the verbosity level of the Java and LLVM simulators.

``-v num``, ``--verbose=num``
    Set the verbosity level of the SAWScript interpreter.


SAW also uses several environment variables for configuration:

``CRYPTOLPATH``
    Specify a colon-delimited list of directory paths to search for Cryptol imports (including the Cryptol prelude).

``SAW_IMPORT_PATH``
    Specify a colon-delimited list of directory paths to search for imports.

``SAW_JDK_JAR``
    Specify the path of the ``.jar`` file containing the core Java libraries.

On Windows, semicolon-delimited lists are used instead of colon-delimited lists.


Structure of SAWScript
----------------------

A SAWScript program consists, at the top level, of a sequence of commands to be executed in order. Each command is terminated with a semicolon. For example, the :saw:ref:`print` command displays a textual representation of its argument. Suppose the following text is stored in the file ``print.saw``:

.. code-block:: SAWScript

    print 3;

The command ``saw print.saw`` will then yield output similar to the following::

  Loading module Cryptol
  Loading file "print.saw"
  3

The same code can be run from the interactive :term:`REPL`::
  sawscript> print 3;
  3

At the REPL, terminating semicolons can be omitted::

  sawscript> print 3
  3

To make common use cases simpler, bare values at the REPL are treated as if they were arguments to print::

  sawscript> 3
  3

One SAWScript file can be included in another using the :saw:ref:`include` command, which takes the name of the file to be included as an argument. For example::

  sawscript> include "print.saw"
  Loading file "print.saw"
  3

Typically, included files are used to import definitions, not perform side effects like printing. However, as you can see, if any commands with side effects occur at the top level of the imported file, those side effects will occur during import.

.. saw:command:: include
  :type: String -> TopLevel ()

  Include another SAWScript file.

.. saw:command:: print
  :type: {a} a -> TopLevel a

  Print the argument to the console.


Syntax
------

The syntax of SAWScript is reminiscent of functional languages such as Cryptol, Haskell and ML. In particular, functions are applied by writing them next to their arguments rather than by using parentheses and commas. Rather than writing ``f(x, y)``, write ``f x y``.

Comments are written as in C and Java (among many other languages). All text from ``//`` until the end of a line is ignored. Additionally, all text between ``/*`` and ``*/`` is ignored, regardless of whether the line ends.

Basic Types and Values
----------------------

All values in SAWScript have types, and these types are determined and checked before a program runs (that is, SAWScript is statically typed). The basic types available are similar to those in many other languages.

.. saw:type:: Int

The ``Int`` type represents unbounded mathematical integers. Integer constants can be written in decimal notation (e.g., ``42``), hexadecimal notation (``0x2a``), and binary (``0b00101010``). However, unlike many languages, integers in SAWScript are used primarily as constants. Arithmetic is usually encoded in Cryptol, as discussed in the next section.


.. saw:type:: Bool
  :constructors: true false

  The Boolean type contains the values true and false, like in many other languages. As with integers, computations on Boolean values usually occur in Cryptol.


.. saw:type:: Tuples

Values of any type can be aggregated into tuples. For example, the value ``(true, 10)`` has the type ``(Bool, Int)``.


.. saw:type:: Records

Values of any type can also be aggregated into records, which are exactly like tuples except that their components have names. For example, the value ``{ b = true, n = 10 }`` has the type ``{ b : Bool, n : Int }``.

.. saw:type:: Lists

A sequence of values of the same type can be stored in a list. For example, the value ``[true, false, true]`` has the type ``[Bool]``.


.. saw:type:: String

  Strings of textual characters can be represented in the String type. For example, the value "example" has type String.


.. saw:type:: ()

The "unit" type, written ``()``, is essentially a placeholder, similar to ``void`` in languages like C and Java. It has only one value, also written ``()``. Values of type ``()`` convey no information. We will show in later sections several cases where this is useful.


.. saw:type:: ->

Functions are given types that indicate what type they consume and what type they produce. For example, the type ``Int -> Bool`` indicates a function that takes an ``Int`` as input and produces a ``Bool`` as output. Functions with multiple arguments use multiple arrows. For example, the type ``Int -> String -> Bool`` indicates a function in which the first argument is an ``Int``, the second is a ``String``, and the result is a ``Bool``. It is possible, but not necessary, to group arguments in tuples, as well, so the type ``(Int, String) -> Bool`` describes a function that takes one argument, a pair of an ``Int`` and a ``String``, and returns a ``Bool``.


SAWScript also includes some more specialized types that do not have straightforward counterparts in most other languages. These will appear in later sections.


Basic Expression Forms
----------------------

One of the key forms of top-level command in SAWScript is a :term:`binding`, introduced with the ``let`` keyword, which gives a name to a value. For example::

    sawscript> let x = 5
    sawscript> x
    5


Bindings can have parameters, in which case they define functions. For instance, the following function takes one parameter and constructs a list containing that parameter as its single element::

    sawscript> let f x = [x]
    sawscript> f "text"
    ["text"]


.. saw:syntax:: let

  ``let x = e`` defines ``x`` to mean ``e``. With arguments, ``let f arg ... = e`` defines ``f`` to be a function with arguments ``arg ...``, returning ``e``.


Functions themselves are values and have types. The type of a function that takes an argument of type ``a`` and returns a result of type ``b`` is ``a -> b``.

Function types are typically inferred, as in the example ``f`` above. In this case, because ``f`` only creates a list with the given argument, and because it is possible to create a list of any element type, ``f`` can be applied to an argument of any type. We say, therefore, that ``f`` is polymorphic. Concretely, we write the type of ``f`` as ``{a} a -> [a]``, meaning it takes a value of any type (denoted ``a``) and returns a list containing elements of that same type. This means we can also apply ``f`` to ``10``::

    sawscript> f 10
    [10]

However, we may want to specify that a function has a more specific type. In this case, we could restrict ``f`` to operate only on :saw:ref:`Int` parameters::

    sawscript> let f (x : Int) = [x]



This will work identically to the original ``f`` on an :saw:ref:`Int` parameter::

    sawscript> f 10
    [10]



However, it will fail for a :saw:ref:`String` parameter::

    sawscript> f "text"

    type mismatch: String -> t.0 and Int -> [Int]
     at "_" (REPL)
    mismatched type constructors: String and In


.. index:: type annotation

Type annotations can be applied to any expression. The notation ``(e : t)`` indicates that expression ``e`` is expected to have type ``t`` and that it is an error for ``e`` to have a different type. Most types in SAWScript are inferred automatically, but specifying them explicitly can sometimes enhance readability.



Because functions are values, functions can return other functions. We make use of this feature when writing functions of multiple arguments. Consider the function ``g``, similar to ``f`` but with two arguments::

    sawscript> let g x y = [x, y]

Like ``f``, ``g`` is polymorphic. Its type is ``{a} a -> a -> [a]``. This means it takes an argument of type ``a`` and returns a function that takes an argument of the same type ``a`` and returns a list of ``a`` values. We can therefore apply ``g`` to any two arguments of the same type::

    sawscript> g 2 3
    [2,3]
    sawscript> g true false
    [true,false]

But type checking will fail if we apply it to two values of different types::

    sawscript> g 2 false

    type mismatch: Bool -> t.0 and Int -> [Int]
     at "_" (REPL)
    mismatched type constructors: Bool and Int

.. index:: function
.. index:: command

So far we have used two related terms, *function* and *command*, and we take these to mean slightly different things. A function is any value with a function type (e.g., ``Int -> [Int]``). A command is any value with a special command type (e.g. ``TopLevel ()``, as shown below). These special types allow us to restrict command usage to specific contexts, and are also *parameterized* (like the list type). Most but not all commands are also functions.

.. saw:type:: TopLevel

  Indicates commands that can run at the top level (directly at the :term:`REPL`, or as one of the top level commands in a script file). The type ``TopLevel a`` represents a command that returns a value of type ``a``.

The :saw:ref:`print` command has the type ``{a} a -> TopLevel ()``, where ``TopLevel ()`` means that it is a command that runs in the `TopLevel` context and returns a value of type :saw:ref:`()` (that is, no useful information). In other words, it has a side effect (printing some text to the screen) but doesn’t produce any information to use in the rest of the SAWScript program. This is the primary usage of the :saw:ref:`()` type.

It can sometimes be useful to bind a sequence of commands together. This can be accomplished with the ``do { ... }`` construct. For example::

    sawscript> let print_two = do { print "first"; print "second"; }
    sawscript> print_two
    first
    second

The bound value, ``print_two``, has type ``TopLevel ()``, since that is the type of its last command.

.. saw:syntax:: do
  ``do { cmd ... }`` constructs a compound command that runs ``cmd ...`` in sequence.

Note that in the previous example the printing doesn’t occur until ``print_two`` directly appears at the :term:`REPL`. The ``let`` expression does not cause those commands to run. The construct that runs a command is written using the ``<-`` operator. This operator works like ``let`` except that it says to run the command listed on the right hand side and bind the result, rather than binding the variable to the command itself. Using ``<-`` instead of ``let`` in the previous example yields::

    sawscript> print_two <- do { print "first"; print "second"; }
    first
    second
    sawscript> print print_two
    ()

Here, the :saw:ref:`print` commands run first, and then ``print_two`` gets the value returned by the second :saw:ref:`print` command, namely :saw:ref:`()`. Any command run without using ``<-`` at either the top level of a script or within a ``do`` block discards its result. However, the :term:`REPL` prints the result of any command run without using the ``<-`` operator.

In some cases it can be useful to have more control over the value returned by a ``do`` block. The :saw:ref:`return` command allows us to do this. For example, say we wanted to write a function that would print a message before and after running some arbitrary command and then return the result of that command. We could write:

.. code-block:: SAWScript

  let run_with_message msg c =
    do {
      print "Starting.";
      print msg;
      res <- c;
      print "Done.";
      return res;
    };

  x <- run_with_message "Hello" (return 3);
  print x;



If we put this script in ``run.saw`` and run it with ``saw``, we get something like::

    Loading module Cryptol
    Loading file "run.saw"
    Starting.
    Hello
    Done.
    3

Note that it ran the first :saw:ref:`print` command, then the caller-specified command, then the second :saw:ref:`print` command. The result stored in ``x`` at the end is the result of the :saw:ref:`return` command passed in as an argument.

.. saw:command:: return
  
  A command that just returns a value, doing nothing else.


Other Basic Functions
---------------------

Aside from the functions we have listed so far, there are a number of other operations for working with basic data structures and interacting with the operating system.


.. saw:function:: concat
  :type: {a} [a] -> [a] -> [a]

  Concatenate two lists.

.. saw:function:: head
  :type: {a} [a] -> a

  Return the first element of a list.

.. saw:function:: tail
  :type: {a} [a] -> [a]

  Return everything in a list except the first element.

.. saw:function:: length
  :type: {a} [a] -> Int

  Counts the number of elements in a list.

.. saw:function:: null
  :type: {a} [a] -> Bool

  Indicates whether a list is empty (has zero elements).

.. saw:function:: nth
  :type: {a} [a] -> Int -> a

  Returns the element at the given position, with ``nth l 0`` being equivalent to ``head l``.

.. saw:function:: for
  :type: {m, a, b} [a] -> (a -> m b) -> m [b]
  
  Takes a list and a function that runs in some command context. The passed command will be called once for every element of the list, in order. Returns a list of all of the results produced by the command.


For interacting with the operating system, we have:

.. saw:function:: get_opt
  :type: Int -> String

  Get the command-line argument to ``saw`` at the given index. Argument 0 is always the name of the ``saw`` executable itself, and higher indices represent later arguments.

.. saw:command:: exec
  :type: String -> [String] -> String -> TopLevel String

  Runs an external program given, respectively, an executable name, a list of arguments, and a string to send to the standard input of the program. The :saw:ref:`exec` command returns the standard output from the program it executes and prints standard error to the screen.

.. saw:command:: exit
  :type: Int -> TopLevel ()

  Stops execution of the current script and returns the given exit code to the operating system.


     

Finally, there are a few miscellaneous functions and commands:

.. saw:function:: show
  :type: {a} a -> String

  Computes the textual representation of its argument in the same way as print, but instead of displaying the value it returns it as a String value for later use in the program. This can be useful for constructing more detailed messages later.

.. saw:function:: str_concat
  :type: String -> String -> String

  Concatenates two :saw:ref:`String` values, and can also be useful with :saw:ref:`show`.

.. saw:function:: time
  :type: {a} TopLevel a -> TopLevel a

  Runs any other :saw:ref:`TopLevel` command and prints out the time it took to execute.

.. saw:function:: with_time
  :type: {a} TopLevel a -> TopLevel (Int, a)

  Returns both the original result of the timed command and the time taken to execute it (in milliseconds), without printing anything in the process.


The Term Type
-------------

Perhaps the most important type in SAWScript, and the one most unlike the built-in types of most other languages, is the :saw:ref:`Term` type. Essentially, a value of type :saw:ref:`Term` precisely describes all possible computations performed by some program. In particular, if two :saw:ref:`Term` values are equivalent, then the programs that they represent will always compute the same results given the same inputs. We will say more later about exactly what it means for two terms to be equivalent, and how to determine whether two terms are equivalent.

Before exploring the :saw:ref:`Term` type more deeply, it is important to understand the role of the :term:`Cryptol` language in SAW.

.. saw:type:: Term

  A value of type Term precisely describes all possible computations performed by some program.


Cryptol and its Role in SAW
---------------------------

Cyptol is a domain-specific language originally designed for the high-level specification of cryptographic algorithms. It is general enough, however, to describe a wide variety of programs, and is particularly applicable to describing computations that operate on streams of data of some fixed size.

.. index:: Cryptol

In addition to being integrated into SAW, Cryptol is a standalone language with `its own manual <http://cryptol.net/files/ProgrammingCryptol.pdf>`_.

SAW includes deep support for Cryptol, and in fact requires the use of Cryptol for most non-trivial tasks. To fully understand the rest of this manual and to effectively use SAW, you will need to develop at least a rudimentary understanding of Cryptol.


The primary use of Cryptol within SAWScript is to construct values of type :saw:ref:`Term`. Although :saw:ref:`Term` values can be constructed from various sources, inline Cryptol expressions are the most direct and convenient way to create them.

Specifically, a Cryptol expression can be placed inside double curly braces (``{{`` and ``}}``), resulting in a value of type :saw:ref:`Term`. As a very simple example, there is no built-in integer addition operation in SAWScript. However, we can use Cryptol’s built-in integer addition operator within SAWScript as follows:: 

    sawscript> let t = {{ 0x22 + 0x33 }}
    sawscript> print t
    85
    sawscript> :type t
    Term

.. saw:syntax:: {{

  Double braces embed Cryptol into SAWScript, returning a :saw:ref:`Term`.

Although it printed out in the same way as an :saw:ref:`Int`, it is important to note that ``t`` actually has type :saw:ref:`Term`. We can see how this term is represented internally, before being evaluated, with the :saw:ref:`print_term` function::

    sawscript> print_term t
    let { x@1 = Prelude.Vec 8 Prelude.Bool
          x@2 = Cryptol.TCNum 8
          x@3 = Cryptol.PLiteralSeqBool x@2
        }
     in Cryptol.ecPlus x@1 (Cryptol.PArithSeqBool x@2)
          (Cryptol.ecNumber (Cryptol.TCNum 34) x@1 x@3)
          (Cryptol.ecNumber (Cryptol.TCNum 51) x@1 x@3)


For the moment, it’s not important to understand what this output means. We show it only to clarify that ``Term`` values have their own internal structure that goes beyond what exists in SAWScript. The internal representation of ``Term`` values is in a language called SAWCore. The full semantics of SAWCore are beyond the scope of this manual.

.. saw:command:: print_term
  :type: Term -> TopLevel ()

  Print a term without evaluating it.



The text constructed by :saw:ref:`print_term` can also be accessed programmatically (instead of printing to the screen) using the :saw:ref:`show_term` function, which returns a :saw:ref:`String`. The :saw:ref:`show_term` function is not a command, so it executes directly and does not need ``<-`` to bind its result. Therefore, the following will have the same result as the :saw:ref:`print_term` command above::

    sawscript> let s = show_term t
    sawscript> :type s
    String
    sawscript> print s
    <same as above>

.. saw:function:: show_term
  :type: Term -> String

  Return the string that would have been displayed by :saw:ref:`print_term`.
  



Numbers are printed in decimal notation by default when printing terms, but the following two commands can change that behavior:



.. saw:command:: set_ascii
  :type: Bool -> TopLevel ()

  When passed true, ``set_ascii`` makes subsequent :saw:ref:`print_term` or :saw:ref:`show_term` commands print sequences of bytes as ASCII strings (and doesn’t affect printing of anything else).

.. saw:command:: set_base
  :type: Int -> TopLevel ()

  Prints all bit vectors in the given base, which can be between 2 and 36 (inclusive).


A :saw:ref:`Term` that represents an integer (any bit vector, as affected by :saw:ref:`set_base`) can be translated into a SAWScript :saw:ref:`Int` using the ``eval_int`` function. This function returns an Int if the Term can be represented as one, and fails at runtime otherwise::

    sawscript> print (eval_int t)
    85
    sawscript> print (eval_int {{ True }})

    "eval_int" (<stdin>:1:1):
    eval_int: argument is not a finite bitvector
    sawscript> print (eval_int {{ [True] }})
    1


.. saw:function:: eval_int
  :type: Term -> Int

  Returns an :saw:ref:`Int` if the :saw:ref:`Term` can be represented as one, and fails at runtime otherwise.


Similarly, values of type ``Bit`` in Cryptol can be translated into values of type Bool in SAWScript using the :saw:ref:`eval_bool` function::

    sawscript> let b = {{ True }}
    sawscript> print_term b
    Prelude.True
    sawscript> print (eval_bool b)
    true

.. saw:function:: eval_bool
  :type: Term -> Bool

  Returns a :saw:ref:`Bool` if the :saw:ref:`Term` can be represented as one, and fails at runtime otherwise.


Anything with sequence type in Cryptol can be translated into a list of :saw:ref:`Term` values in SAWScript using the :saw:ref:`eval_list` function::

    sawscript> let l = {{ [0x01, 0x02, 0x03] }}
    sawscript> print_term l
    let { x@1 = Prelude.Vec 8 Prelude.Bool
          x@2 = Cryptol.PLiteralSeqBool (Cryptol.TCNum 8)
        }
     in [Cryptol.ecNumber (Cryptol.TCNum 1) x@1 x@2
        ,Cryptol.ecNumber (Cryptol.TCNum 2) x@1 x@2
        ,Cryptol.ecNumber (Cryptol.TCNum 3) x@1 x@2]
    sawscript> print (eval_list l)
    [Cryptol.ecNumber (Cryptol.TCNum 1) (Prelude.Vec 8 Prelude.Bool)
      (Cryptol.PLiteralSeqBool (Cryptol.TCNum 8))
    ,Cryptol.ecNumber (Cryptol.TCNum 2) (Prelude.Vec 8 Prelude.Bool)
      (Cryptol.PLiteralSeqBool (Cryptol.TCNum 8))
    ,Cryptol.ecNumber (Cryptol.TCNum 3) (Prelude.Vec 8 Prelude.Bool)
      (Cryptol.PLiteralSeqBool (Cryptol.TCNum 8))]

.. saw:function:: eval_list
  :type: Term -> [Term]

  Return a list of :saw:ref:`Term` values if the given :saw:ref:`Term` represents a Cryptol sequence.

Finally, a list of :saw:ref:`Term` values in SAWScript can be collapsed into a single :saw:ref:`Term` with sequence type using the :saw:ref:`list_term` function, which is the inverse of :saw:ref:`eval_list`::

    sawscript> let ts = eval_list l
    sawscript> let l = list_term ts
    sawscript> print_term l
    let { x@1 = Prelude.Vec 8 Prelude.Bool
          x@2 = Cryptol.PLiteralSeqBool (Cryptol.TCNum 8)
        }
     in [Cryptol.ecNumber (Cryptol.TCNum 1) x@1 x@2
        ,Cryptol.ecNumber (Cryptol.TCNum 2) x@1 x@2
        ,Cryptol.ecNumber (Cryptol.TCNum 3) x@1 x@2]

.. saw:function:: list_term
  :type: [Term] -> Term

  Collapse a list of :saw:ref:`Term` values into a single :saw:ref:`Term` with a sequence type.



In addition to being able to extract integer and Boolean values from Cryptol expressions, :saw:ref:`Term` values can be injected into Cryptol expressions. When SAWScript evaluates a Cryptol expression between ``{{``  and ``}}`` delimiters, it does so with several extra bindings in scope:

* Any variable in scope that has SAWScript type :saw:ref:`Bool` is visible in Cryptol expressions as a value of type ``Bit``.

* Any variable in scope that has SAWScript type :saw:ref:`Int` is visible in Cryptol expressions as a type variable. Type variables can be demoted to numeric bit vector values using the backtick (`````) operator.

* Any variable in scope that has SAWScript type :saw:ref:`Term` is visible in Cryptol expressions as a value with the Cryptol type corresponding to the internal type of the term. The power of this conversion is that the :saw:ref:`Term` does not need to have originally been derived from a Cryptol expression.

In addition to these rules, bindings created at the Cryptol level, either from included files or inside Cryptol quoting brackets, are visible only to later Cryptol expressions, and not as SAWScript variables.

To make these rules more concrete, consider the following examples. If we bind a SAWScript :saw:ref:`Int`, we can use it as a Cryptol type variable. If we create a :saw:ref:`Term` variable that internally has function type, we can apply it to an argument within a Cryptol expression, but not at the SAWScript level::

    sawscript> let n = 8
    sawscript> :type n
    Int
    sawscript> let {{ f (x : [n]) = x + 1 }}
    sawscript> :type {{ f }}
    Term
    sawscript> :type f

    <stdin>:1:1-1:2: unbound variable: "f" (<stdin>:1:1-1:2)
    sawscript> print {{ f 2 }}
    3

If ``f`` was a binding of a SAWScript variable to a :saw:ref:`Term` of function type, we would get a different error::

    sawscript> let f = {{ \(x : [n]) -> x + 1 }}
    sawscript> :type {{ f }}
    Term
    sawscript> :type f
    Term
    sawscript> print {{ f 2 }}
    3
    sawscript> print (f 2)

    type mismatch: Int -> t.0 and Term
     at "_" (REPL)
     mismatched type constructors: (->) and Term

One subtlety of dealing with a :saw:ref:`Term` constructed from Cryptol is that because the Cryptol expressions themselves are type checked by the Cryptol type checker, and because they may make use of other :saw:ref:`Term` values already in scope, they are not type checked until the Cryptol brackets are evaluated. So type errors at the Cryptol level may occur at runtime from the SAWScript perspective (though they occur before the Cryptol expressions are run).


So far, we have talked about using Cryptol value expressions. However, SAWScript can also work with Cryptol types. The most direct way to refer to a Cryptol type is to use type brackets: ``{|`` and ``|}``. Any Cryptol type written between these brackets becomes a :saw:ref:`Type` value in SAWScript. Some types in Cryptol are numeric (also known as size) types, and correspond to non-negative integers. These can be translated into SAWScript integers with the :saw:ref:`eval_size` function. For example::

    sawscript> let {{ type n = 16 }}
    sawscript> eval_size {| n |}
    16
    sawscript> eval_size {| 16 |}
    16

For non-numeric types, eval_size fails at runtime::

    sawscript> eval_size {| [16] |}

    "eval_size" (<stdin>:1:1):
    eval_size: not a numeric type

.. saw:type:: Type

  A value of type ``Type`` precisely describes a Cryptol type.

.. saw:function:: eval_size
  :type: Type -> Int

  Evaluate a numeric type into an :saw:ref:`Int`, or fail if the type is not numeric.


In addition to the use of brackets to write Cryptol expressions inline, several built-in functions can extract :saw:ref:`Term` values from Cryptol files in other ways. First, the :saw:ref:`import` statement makes all definitions from a Cryptol file available.

.. saw:syntax:: import

  Imports all top-level definitions from a Cryptol file and places them in scope within later bracketed expressions.


The :saw:ref:`cryptol_load` command behaves similarly, but returns a :saw:ref:`CryptolModule` instead. If any :saw:ref:`CryptolModule` is in scope, its contents are available qualified with the name of the :saw:ref:`CryptolModule` variable. A specific definition can be explicitly extracted from a :saw:ref:`CryptolModule` using the :saw:ref:`cryptol_extract` command.

.. saw:command:: cryptol_load
  :type: String -> TopLevel CryptolModule

  Load a Cryptol module similarly to :saw:ref:`import`, but instead of bringing its definitions into scope, store them in a module object.

.. saw:type:: CryptolModule

  A Cryptol module, the result of :saw:ref:`cryptol_load`. If a value of type :saw:ref:`CryptolModule` is in scope, then its name can be used to qualify Cryptol names within Cryptol expressions in :saw:ref:`{{` blocks, allowing them access to the contents of the module.

.. saw:command:: cryptol_extract
  :type: CryptolModule -> String -> TopLevel Term

  Extract a single Term from a loaded Cryptol module.

Transforming Term Values
------------------------

The three primary functions of SAW are *extracting* models (:saw:ref:`Term` values) from programs, *transforming* those models, and *proving* properties about models using external provers. So far we’ve shown how to construct :saw:ref:`Term` values from Cryptol programs; later sections will describe how to extract them from other programs. Now we show how to use the various term transformation features available in SAW.

Rewriting
~~~~~~~~~

Rewriting a :saw:ref:`Term` consists of applying one or more rewrite rules to it, resulting in a new :saw:ref:`Term`. A rewrite rule in SAW can be specified in multiple ways:

* as the definition of a function that can be unfolded,
* as a term of Boolean type (or a function returning a Boolean) that is an equality statement, and
* as a term of equality type with a body that encodes a proof that the equality in the type is valid.

In each case the term logically consists of two sides and describes a way to transform the left side into the right side. Each side may contain variables (bound by enclosing lambda expressions) and is therefore a *pattern* which can match any term in which each variable represents an arbitrary sub-term. The left-hand pattern describes a term to match (which may be a sub-term of the full term being rewritten), and the right-hand pattern describes a term to replace it with. Any variable in the right-hand pattern must also appear in the left-hand pattern and will be instantiated with whatever sub-term matched that variable in the original term.

For example, say we have the following Cryptol function::

  \(x:[8]) -> (x * 2) + 1

We might for some reason want to replace multiplication by a power of two with a shift. We can describe this replacement using an equality statement in Cryptol (a rule of form 2 above)::

  \(y:[8]) -> (y * 2) == (y << 1)

Interpreting this as a rewrite rule, it says that for any 8-bit vector (call it ``y`` for now), we can replace ``y * 2`` with ``y << 1``. Using this rule to rewrite the earlier expression would then yield::

  \(x:[8]) -> (x << 1) + 1


The general philosophy of rewriting is that the left and right patterns, while syntactically different, should be semantically equivalent. Therefore, applying a set of rewrite rules should not change the fundamental meaning of the term being rewritten. SAW is particularly focused on the task of proving that some logical statement expressed as a :saw:ref:`Term` is always true. If that is in fact the case, then the entire term can be replaced by the term ``True`` without changing its meaning. The rewriting process can in some cases, by repeatedly applying rules that themselves are known to be valid, reduce a complex term entirely to True, which constitutes a proof of the original statement. In other cases, rewriting can simplify terms before sending them to external automated provers that can then finish the job. Sometimes this simplification can help the automated provers run more quickly, and sometimes it can help them prove things they would otherwise be unable to prove by applying reasoning steps (rewrite rules) that are not available to the automated provers.

.. saw:type:: Simpset

  A collection of rewrite rules.

In practical use, rewrite rules can be aggregated into :saw:ref:`Simpset` values in SAWScript. A few pre-defined :saw:ref:`Simpset` values exist:

.. saw:value:: empty_ss
  :type: Simpset

  The empty set of rules. Rewriting with it should have no effect, but it is useful as an argument to some of the functions that construct larger Simpset values.

.. saw:value:: basic_ss
  :type: Simpset 

  A collection of rules that are useful in most proof scripts.

.. saw:function:: cryptol_ss
  :type: () -> Simpset

   Includes a collection of Cryptol-specific rules. Some of these simplify away the abstractions introduced in the translation from Cryptol to SAWCore, which can be useful when proving equivalence between Cryptol and non-Cryptol code. Leaving these abstractions in place is appropriate when comparing only Cryptol code, however, so :saw:ref:`cryptol_ss` is not included in :saw:ref:`basic_ss`.

The next set of functions can extend or apply a Simpset:

.. saw:function:: addsimp'
  :type: Term -> Simpset -> Simpset

  Adds a single Term to an existing :saw:ref:`Simpset`.

.. saw:function:: addsimps'
  :type: [Term] -> Simpset -> Simpset

  Adds a list of :saw:ref:`Term` objects to an existing :saw:ref:`Simpset`.

.. saw:function:: rewrite
  :type: Simpset -> Term -> Term

  Applies a :saw:ref:`Simpset` to an existing :saw:ref:`Term` to produce a new :saw:ref:`Term`.


To make this more concrete, we examine how the rewriting example sketched above, to convert multiplication into shift, can work in practice. We simplify everything with :saw:ref:`cryptol_ss` as we go along so that the :saw:ref:`Term` values don’t get too cluttered. First, we declare the term to be transformed::

    sawscript> let term = rewrite (cryptol_ss ()) {{ \(x:[8]) -> (x * 2) + 1 }}
    sawscript> print_term term
    \(x : Prelude.Vec 8 Prelude.Bool) ->
      Prelude.bvAdd 8 (Prelude.bvMul 8 x (Prelude.bvNat 8 2))
        (Prelude.bvNat 8 1)

Next, we declare the rewrite rule::

    sawscript> let rule = rewrite (cryptol_ss ()) {{ \(y:[8]) -> (y * 2) == (y << 1) }}
    sawscript> print_term rule
    let { x@1 = Prelude.Vec 8 Prelude.Bool
        }
     in \(y : x@1) ->
          Cryptol.ecEq x@1 (Cryptol.PCmpWord 8)
            (Prelude.bvMul 8 y (Prelude.bvNat 8 2))
            (Prelude.bvShiftL 8 Prelude.Bool 1 Prelude.False y
               (Prelude.bvNat 1 1))

Finally, we apply the rule to the target term::

    sawscript> let result = rewrite (addsimp' rule empty_ss) term
    sawscript> print_term result
    \(x : Prelude.Vec 8 Prelude.Bool) ->
      Prelude.bvAdd 8
        (Prelude.bvShiftL 8 Prelude.Bool 1 Prelude.False x
           (Prelude.bvNat 1 1))
        (Prelude.bvNat 8 1)

Note that ``addsimp'`` and ``addsimps'`` take a :saw:ref:`Term` or a :saw:ref:`Term` list; these could in principle be anything, and are not necessarily terms representing logically valid equalities. They have ``'`` suffixes because they are not intended to be the primary interface to rewriting. When using these functions, the soundness of the proof process depends on the correctness of these rules as a side condition.

The primary interface to rewriting uses the :saw:ref:`Theorem` type instead of the :saw:ref:`Term` type, as shown in the signatures for :saw:ref:`addsimp` and :saw:ref:`addsimps`.


.. saw:function:: addsimp
  :type: Theorem -> Simpset -> Simpset

  Adds a single :saw:ref:`Theorem` to a :saw:ref:`Simpset`.

.. saw:function:: addsimps
  :type: [Theorem] -> Simpset -> Simpset

  Adds several :saw:ref:`Theorem` values to a :saw:ref:`Simpset`.

A :saw:ref:`Theorem` is essentially a :saw:ref:`Term` that is proven correct in some way. In general, a :saw:ref:`Theorem` can be any statement, and may not be useful as a rewrite rule. However, if it has an appropriate shape it can be used for rewriting. In the "Proofs about Terms" section, we’ll describe how to construct :saw:ref:`Theorem` values from :saw:ref:`Term` values.

.. saw:type:: Theorem

  A :saw:ref:`Term` that has been proven correct in some way.

In the absence of user-constructed :saw:ref:`Theorem` values, there are some additional built-in rules that are not included in either :saw:ref:`basic_ss` and :saw:ref:`cryptol_ss` because they are not always beneficial, but that can sometimes be helpful or essential. The :saw:ref:`cryptol_ss` simpset includes rewrite rules to unfold all definitions in the Cryptol SAWCore module, but does not include any of the terms of equality type.

.. saw:function:: add_cryptol_eqs
  :type: [String] -> Simpset -> Simpset

  Adds the terms of equality type with the given names from the SAWCore Cryptol module to the given :saw:ref:`Simpset`.

.. saw:function:: add_prelude_defs
  :type: [String] -> Simpset -> Simpset

  Adds unfolding rules from the SAWCore Prelude module to a :saw:ref:`Simpset`.

.. saw:function:: add_prelude_eqs
  :type: [String] -> Simpset -> Simpset

  Adds equality-typed term from the SAWCore Prelude module to a :saw:ref:`Simpset`.

Finally, it’s possible to construct a theorem from an arbitrary SAWCore expression (rather than a Cryptol expression), using the :saw:ref:`core_axiom` function.


.. saw:function:: core_axiom
  :type: String -> Theorem

  Creates a :saw:ref:`Theorem` from a :saw:ref:`String` in SAWCore syntax. Any :saw:ref:`Theorem` introduced by this function is assumed to be correct, so use it with caution.


Folding and Unfolding
~~~~~~~~~~~~~~~~~~~~~


A SAWCore term can be given a name using the :saw:ref:`define` function, and is then by default printed as that name alone. A named subterm can be "unfolded" so that the original definition appears again.


.. saw:command:: define
  :type: String -> Term -> TopLevel Term

  The arguments are a name and a term. Executing this command results in the name being defined to mean the term. The result of the command is a :saw:ref:`Term` that contains a reference to the defined name.

.. saw:function:: unfold_term
  :type: [String] -> Term -> Term

  Unfold any of the provided names that appear in the given :saw:ref:`Term`, replacing the defined name with its definition.

For example::

    sawscript> let t = {{ 0x22 }}
    sawscript> print_term t
    Cryptol.ecNumber (Cryptol.TCNum 34) (Prelude.Vec 8 Prelude.Bool)
      (Cryptol.PLiteralSeqBool (Cryptol.TCNum 8))
    sawscript> t' <- define "t" t
    sawscript> print_term t'
    t
    sawscript> let t'' = unfold_term ["t"] t'
    sawscript> print_term t''
    Cryptol.ecNumber (Cryptol.TCNum 34) (Prelude.Vec 8 Prelude.Bool)
      (Cryptol.PLiteralSeqBool (Cryptol.TCNum 8))



This process of folding and unfolding is useful both to make large terms easier for humans to work with and to make automated proofs more tractable. We’ll describe the latter in more detail when we discuss interacting with external provers.

In some cases, folding happens automatically when constructing Cryptol expressions. Consider the following example::

    sawscript> let t = {{ 0x22 }}
    sawscript> print_term t
    Cryptol.ecNumber (Cryptol.TCNum 34) (Prelude.Vec 8 Prelude.Bool)
      (Cryptol.PLiteralSeqBool (Cryptol.TCNum 8))
    sawscript> let {{ t' = 0x22 }}
    sawscript> print_term {{ t' }}
    t'

This illustrates that a bare expression in Cryptol braces gets translated directly to a SAWCore term. However, a Cryptol *definition* gets translated into a *folded* SAWCore term. In addition, because the second definition of ``t`` occurs at the Cryptol level, rather than the SAWScript level, it is visible only inside Cryptol braces. Definitions imported from Cryptol source files are also initially folded and can be unfolded as needed.

Other Built-in Transformation and Inspection Functions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In addition to the :saw:ref:`Term` transformation functions described so far, a variety of others also exist.



.. saw:function:: beta_reduce_term
  :type: Term -> Term

  Replaces any sub-expression of the form ``(\x -> t) v`` in the given :saw:ref:`Term` with a transformed version of ``t`` in which all instances of ``x`` are replaced by ``v`` .

.. saw:command:: replace
  :type: Term -> Term -> Term -> TopLevel Term

  Replaces arbitrary subterms. A call to ``replace x y t`` replaces any instance of ``x`` inside ``t`` with ``y``.


     

Assessing the size of a term can be particularly useful during benchmarking. SAWScript provides two mechanisms for this.

.. saw:function:: term_size
  :type: Term -> Int

  Calculates the number of nodes in the Directed Acyclic Graph (DAG) representation of a :saw:ref:`Term` used internally by SAW. This is the most appropriate way of determining the resource use of a particular term.

.. saw:function:: term_tree_size
  :type: Term -> Int

  Calculates how large a Term would be if it were represented by a tree instead of a DAG. This can, in general, be much, much larger than the number returned by :saw:ref:`term_size`, and serves primarily as a way of assessing, for a specific term, how much benefit there is to the term sharing used by the DAG representation.

Finally, there are a few commands related to the internal SAWCore type of a :saw:ref:`Term`.

.. saw:command:: check_term
  :type: Term -> TopLevel ()

  Checks that the internal structure of a Term is well-formed and that it passes all of the rules of the SAWCore type checker.

.. saw:function:: type
  :type: Term -> Type

  Returns the type of a particular :saw:ref:`Term`, which can then be used to, for example, construct a new fresh variable with :saw:ref:`fresh_symbolic`.


Loading and Storing Terms
~~~~~~~~~~~~~~~~~~~~~~~~~

Most frequently, :saw:ref:`Term` values in SAWScript come from Cryptol, JVM, or LLVM programs, or some transformation thereof. However, it is also possible to obtain them from various other sources.



.. saw:function:: parse_core
  :type: String -> Term

  Parses a String containing a term in SAWCore syntax, returning a :saw:ref:`Term`.

.. saw:command:: read_core
  :type: String -> TopLevel Term

   A version of :saw:ref:`parse_core` that obtains text from the given file and expects it to be in the simpler SAWCore external representation format, rather than the human-readable syntax shown so far.

.. saw:command:: read_aig
  :type: String -> TopLevel Term

  Returns a :saw:ref:`Term` representation of an And-Inverter-Graph (AIG) file in AIGER format.

.. saw:command:: read_bytes
  :type: String -> TopLevel Term

  Reads a constant sequence of bytes from a file and represents it as a :saw:ref:`Term`. Its result will always have Cryptol type ``[n][8]`` for some ``n``.


     

It is also possible to write :saw:ref:`Term` values into files in various formats, including: AIGER (:saw:ref:`write_aig`), CNF (:saw:ref:`write_cnf`), SAWCore external representation (:saw:ref:`write_core`), and SMT-Lib version 2 (:saw:ref:`write_smtlib2`).

.. saw:command:: write_aig
  :type: String -> Term -> TopLevel ()

.. saw:command:: write_cnf
  :type: String -> Term -> TopLevel ()

.. saw:command:: write_core
  :type: String -> Term -> TopLevel ()

.. saw:command:: write_smtlib2
  :type: String -> Term -> TopLevel ()


Proofs about Terms
------------------



The goal of SAW is to facilitate proofs about the behavior of programs. It may be useful to prove some small fact to use as a rewrite rule in later proofs, but ultimately these rewrite rules come together into a proof of some higher-level property about a software system.

Whether proving small lemmas (in the form of rewrite rules) or a top-level theorem, the process builds on the idea of a *proof script* that is run by one of the top level proof commands.


.. saw:command:: prove_print
  :type: ProofScript SatResult -> Term -> TopLevel Theorem

  Takes a proof script (which we’ll describe next) and a :saw:ref:`Term`. The :saw:ref:`Term` should be of function type with a return value of :saw:ref:`Bool` (``Bit`` at the Cryptol level). It will then use the proof script to attempt to show that the :saw:ref:`Term` returns ``True`` for all possible inputs. If it is successful, it will print ``Valid`` and return a :saw:ref:`Theorem`. If not, it will abort.

.. saw:command:: sat_print
  :type: ProofScript SatResult -> Term -> TopLevel ()

  A version of :saw:ref:`prove_print` that looks for only a single value for which the :saw:ref:`Term` evaluates to ``True`` and prints out that value, returning nothing.

.. saw:command:: prove_core
  :type: ProofScript SatResult -> String -> TopLevel Theorem

  Proves and returns a :saw:ref:`Theorem` from a string in SAWCore syntax.

.. saw:type:: ProofScript

  A proof script is a description of a method for accomplishing a proof. Proof scripts include calls to solvers, like :saw:ref:`abc`, as well as manual proof steps.

Automated Tactics
~~~~~~~~~~~~~~~~~

The simplest proof scripts just specify the automated prover to use. The :saw:ref:`ProofScript` values :saw:ref:`abc` and :saw:ref:`z3` select the ABC and Z3 theorem provers, respectively, and are typically good choices.


For example, combining :saw:ref:`prove_print` with :saw:ref:`abc`::

    sawscript> t <- prove_print abc {{ \(x:[8]) -> x+x == x*2 }}
    Valid
    sawscript> t
    Theorem (let { x@1 = Prelude.Vec 8 Prelude.Bool
          x@2 = Cryptol.TCNum 8
          x@3 = Cryptol.PArithSeqBool x@2
        }
     in (x : x@1)
    -> Prelude.EqTrue
         (Cryptol.ecEq x@1 (Cryptol.PCmpSeqBool x@2)
            (Cryptol.ecPlus x@1 x@3 x x)
            (Cryptol.ecMul x@1 x@3 x
               (Cryptol.ecNumber (Cryptol.TCNum 2) x@1
                  (Cryptol.PLiteralSeqBool x@2)))))

Similarly, :saw:ref:`sat_print` will show that the function returns ``True`` for one specific input (which it should, since we already know it returns ``True`` for all inputs)::

    sawscript> sat_print abc {{ \(x:[8]) -> x+x == x*2 }}
    Sat: [x = 0]

In addition to these, the :saw:ref:`boolector`, :saw:ref:`cvc4`, :saw:ref:`mathsat`, and :saw:ref:`yices` provers are available. The internal decision procedure :saw:ref:`rme`, short for Reed-Muller Expansion, is an automated prover that works particularly well on the Galois field operations that show up, for example, in AES.

.. saw:value:: abc
  :type: ProofScript

  A proof script that calls the `ABC theorem prover <https://github.com/berkeley-abc/abc>`_.

.. saw:value:: boolector
  :type: ProofScript

  A proof script that calls the `Boolector theorem prover <https://boolector.github.io/>`_.

.. saw:value:: cvc4
  :type: ProofScript

  A proof script that calls the `CVC4 theorem prover <https://cvc4.github.io/>`_.

.. saw:value:: mathsat
  :type: ProofScript

  A proof script that calls the `Mathsat theorem prover <https://mathsat.fbk.eu/>`_.

.. saw:value:: rme
  :type: ProofScript

  An internal decision procedure that uses `Reed-Muller Expansion <https://en.wikipedia.org/wiki/Reed%E2%80%93Muller_expansion>`_.

.. saw:value:: yices
  :type: ProofScript

  A proof script that calls the `Yices theorem prover <https://yices.csl.sri.com/>`_.

.. saw:value:: z3
  :type: ProofScript

  A proof script that calls the `Z3 theorem prover <https://rise4fun.com/z3>`_.




In more complex cases, some pre-processing can be helpful or necessary before handing the problem off to an automated prover. The pre-processing can involve rewriting, beta reduction, unfolding, the use of provers that require slightly more configuration, or the use of provers that do very little real work.


Proof Script Diagnostics
~~~~~~~~~~~~~~~~~~~~~~~~

During development of a proof, it can be useful to print various information about the current goal. The following tactics are useful in that context.



.. saw:command:: print_goal
  :type: ProofScript ()

  Prints the entire goal in SAWCore syntax.

.. saw:command:: print_goal_consts
  :type: ProofScript ()

  Prints a list of unfoldable constants in the current goal.

.. saw:command:: print_goal_depth
  :type: Int -> ProofScript ()

  Takes an integer argument, ``n``, and prints the goal up to depth ``n``. Any elided subterms are printed with a ``...`` notation.

.. saw:command:: print_goal_size
  :type: ProofScript () 
  
  Prints the number of nodes in the DAG representation of the goal.


Rewriting in Proof Scripts
~~~~~~~~~~~~~~~~~~~~~~~~~~

One of the key techniques available for completing proofs in SAWScript is the use of rewriting or transformation. The following commands support this approach.



.. saw:command:: simplify
  :type: Simpset -> ProofScript ()

  Just like :saw:ref:`rewrite`, except that it works in a :saw:ref:`ProofScript` context and implicitly transforms the current (unnamed) goal rather than taking a :saw:ref:`Term` as a parameter.

.. saw:command:: goal_eval
  :type: ProofScript ()

  Evaluates the current proof goal to a first-order combination of primitives.

.. saw:command:: goal_eval_unint
  :type: [String] -> ProofScript ()

  Works like :saw:ref:`goal_eval` but avoids expanding or simplifying the given names.

Other Transformations
~~~~~~~~~~~~~~~~~~~~~

Some useful transformations are not easily specified using equality statements, and instead have special tactics.



.. saw:command:: beta_reduce_goal
  :type: ProofScript ()

  Works like :saw:ref:`beta_reduce_term` but on the current goal. It takes any sub-expression of the form ``(\x -> t) v`` and replaces it with a transformed version of ``t`` in which all instances of ``x`` are replaced by ``v``.

.. saw:command:: unfolding
  :type: [String] -> ProofScript ()

  Works like :saw:ref:`unfold_term` but on the current goal.


Using :saw:ref:`unfolding` is mostly valuable for proofs based entirely on rewriting, since the default behavior for automated provers is to unfold everything before sending a goal to a prover. However, with some provers it is possible to indicate that specific named subterms should be represented as uninterpreted functions.

.. saw:command:: unint_cvc4
  :type: [String] -> ProofScript SatResult

.. saw:command:: unint_yices
  :type: [String] -> ProofScript SatResult

.. saw:command:: unint_z3
  :type: [String] -> ProofScript SatResult


The list of :saw:ref:`String` arguments in these cases indicates the names of the subterms to leave folded, and therefore present as uninterpreted functions to the prover. To determine which folded constants appear in a goal, use the :saw:ref:`print_goal_consts` function described above.

Ultimately, we plan to implement a more generic tactic that leaves certain constants uninterpreted in whatever prover is ultimately used (provided that uninterpreted functions are expressible in the prover).

Other External Provers
~~~~~~~~~~~~~~~~~~~~~~

In addition to the built-in automated provers already discussed, SAW supports more generic interfaces to other arbitrary theorem provers supporting specific interfaces.

.. saw:command:: external_aig_solver
  :type: String -> [String] -> ProofScript SatResult

  Supports theorem provers that can take input as a single-output AIGER file. The first argument is the name of the executable to run. The second argument is the list of command-line parameters to pass to that executable. Any element of this list equal to ``"%f"`` will be replaced with the name of the temporary AIGER file generated for the proof goal. The output from the solver is expected to be in DIMACS solution format.

.. saw:command:: external_cnf_solver
  :type: String -> [String] -> ProofScript SatResult

  Works similarly to :saw:ref:`external_aig_solver` but for SAT solvers that take input in DIMACS CNF format and produce output in DIMACS solution format.


Offline Provers
~~~~~~~~~~~~~~~

For provers that must be invoked in more complex ways, or to defer proof until a later time, there are functions to write the current goal to a file in various formats, and then assume that the goal is valid through the rest of the script.

.. saw:command:: offline_aig
  :type: String -> ProofScript SatResult

.. saw:command:: offline_cnf
  :type: String -> ProofScript SatResult

.. saw:command:: offline_extcore
  :type: String -> ProofScript SatResult

.. saw:command:: offline_smtlib2
  :type: String -> ProofScript SatResult

.. saw:command:: offline_unint_smtlib2
  :type: [String] -> String -> ProofScript SatResult

     

These support the AIGER, DIMACS CNF, shared SAWCore, and SMT-Lib v2 formats, respectively. The shared representation for SAWCore is described `in the saw-script repository <https://github.com/GaloisInc/saw-script/blob/master/doc/extcore.md>`_. The :saw:ref:`offline_unint_smtlib2` command represents the folded subterms listed in its first argument as uninterpreted functions.

Finishing Proofs without External Solvers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Some proofs can be completed using unsound placeholders, or using techniques that do not require significant computation.


.. saw:command:: assume_unsat
  :type: ProofScript SatResult

  Indicates that the current goal should be assumed to be unsatisfiable. At the moment, :saw:ref:`crucible_jvm_verify` and :saw:ref:`crucible_llvm_verify` (described below) run their proofs in a satisfiability-checking (negated) context, so :saw:ref:`assume_unsat` indicates that the property being checked should be assumed to be true. This is likely to change in the future.

.. saw:command:: assume_valid
  :type: ProofScript ProofResult

  Indicates that the current goal should be assumed to be valid.

.. saw:command:: quickcheck
  :type: Int -> ProofScript SatResult

  Runs the goal on the given number of random inputs, and succeeds if the result of evaluation is always ``True``. This is unsound, but can be helpful during proof development, or as a way to provide some evidence for the validity of a specification believed to be true but difficult or infeasible to prove.

.. saw:command:: trivial
  :type: ProofScript SatResult

  States that the current goal should be trivially true (i.e., the constant ``True`` or a function that immediately returns ``True``). It fails if that is not the case.


Multiple Goals
~~~~~~~~~~~~~~

The proof scripts shown so far all have a single implicit goal. As in many other interactive provers, however, SAWScript proofs can have multiple goals. The following commands can introduce or work with multiple goals. These are experimental and can be used only after :saw:ref:`enable_experimental` has been called.



.. saw:command:: goal_apply
  :type: Theorem -> ProofScript ()

  Applies a given introduction rule to the current goal. This will result in zero or more new subgoals.

.. saw:command:: goal_assume
  :type: ProofScript Theorem

  Converts the first hypothesis in the current proof goal into a local :saw:ref:`Theorem`.

.. saw:command:: goal_insert
  :type: Theorem -> ProofScript ()

  Inserts a given :saw:ref:`Theorem` as a new hypothesis in the current proof goal.

.. saw:command:: goal_intro
  :type: String -> ProofScript Term

  Introduce a quantified variable in the current proof goal, returning the variable as a :saw:ref:`Term`.

.. saw:command:: goal_when
  :type: String -> ProofScript () -> ProofScript ()

  Run the given proof script only when the goal name contains the given string.

.. saw:command:: split_goal
  :type: ProofScript ()

  Split a goal of the form ``Prelude.and prop1 prop2`` into two separate goals ``prop1`` and ``prop2``.


Proof Failure and Satisfying Assignments
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The :saw:ref:`prove_print` and :saw:ref:`sat_print` commands print out their essential results (potentially returning a :saw:ref:`Theorem` in the case of :saw:ref:`prove_print`). In some cases, though, one may want to act programmatically on the result of a proof rather than displaying it.

The :saw:ref:`prove` and :saw:ref:`sat`:saw:ref:` commands` allow this sort of programmatic analysis of proof results. To allow this, they use two types we haven’t mentioned yet: :saw:ref:`ProofResult` and :saw:ref:`SatResult`. These are different from the other types in SAWScript because they encode the possibility of two outcomes. In the case of :saw:ref:`ProofResult`, a statement may be valid or there may be a counter-example. In the case of :saw:ref:`SatResult`, there may be a satisfying assignment or the statement may be unsatisfiable.


.. saw:command:: prove
  :type: ProofScript SatResult -> Term -> TopLevel ProofResult

  Run a proof script, similarly to :saw:ref:`prove_print`. Return the result, whether it succeeds or fails.

.. saw:type:: ProofResult

  The result of running a proof script. The result is either an indication that the statement is a theorem or a counterexample.

.. saw:command:: sat
  :type: ProofScript SatResult -> Term -> TopLevel SatResult

  Run a proof script, similarly to :saw:ref:`sat_print`. Return the result, whether it succeeds or fails.

.. saw:type:: SatResult

  The result of running a proof script for satisfaction. The result is either an indication that the statement is unsatisfiable or a satisfying assignment.

To operate on these new types, SAWScript includes a pair of functions:



.. saw:function:: caseProofResult
  :type: {b} ProofResult -> b -> (Term -> b) -> b
  
  Takes a ProofResult, a value to return in the case that the statement is valid, and a function to run on the counter-example, if there is one.

.. saw:function:: caseSatResult
  :type: {b} SatResult -> b -> (Term -> b) -> b

  Returns its first argument if the result represents an unsatisfiable statement, or its second argument applied to a satisfying assignment if there is one.

AIG Values and Proofs
~~~~~~~~~~~~~~~~~~~~~

Most SAWScript programs operate on :saw:ref:`Term` values, and in most cases this is the appropriate representation. It is possible, however, to represent the same function that a :saw:ref:`Term` may represent using a different data structure: an And-Inverter-Graph (AIG). An AIG is a representation of a Boolean function as a circuit composed entirely of AND gates and inverters. Hardware synthesis and verification tools, including the ABC tool that SAW has built in, can do efficient verification and particularly equivalence checking on AIGs.

To take advantage of this capability, a handful of built-in commands can operate on AIGs.

.. saw:command:: bitblast
  :type: Term -> TopLevel AIG

  Represents a Term as an AIG by "blasting" all of its primitive operations (things like bit-vector addition) down to the level of individual bits.

.. saw:command:: cec
  :type: AIG -> AIG -> TopLevel ProofResult

  Compares two AIG values, returning a :saw:ref:`ProofResult` representing whether the two are equivalent.

.. saw:command:: load_aig
  :type: String -> TopLevel AIG

  Loads an AIG from an external AIGER file.

.. saw:command:: save_aig
  :type: String -> AIG -> TopLevel ()

  Saves an AIG to an external AIGER file.

.. saw:command:: save_aig_as_cnf
  :type: String -> AIG -> TopLevel ()

  Writes an AIG out in CNF format for input into a standard SAT solver.


Symbolic Execution
------------------

Analysis of Java and LLVM within SAWScript relies heavily on :term:`symbolic execution`, so some background on how this process works can help with understanding the behavior of the available built-in functions.

At the most abstract level, symbolic execution works like normal program execution except that the values of all variables within the program can be arbitrary *expressions*, potentially containing free variables, rather than concrete values. Therefore, each symbolic execution corresponds to some set of possible concrete executions.

As a concrete example, consider the following C program that returns the maximum of two values:

.. code-block:: C

    unsigned int max(unsigned int x, unsigned int y) {
        if (y > x) {
            return y;
        } else {
            return x;
        }
    }

If you call this function with two concrete inputs, like this:

.. code-block:: C

    int r = max(5, 4);

then it will assign the value ``5`` to ``r``. However, we can also consider what it will do for arbitrary inputs. Consider the following example:

.. code-block:: C

    int r = max(a, b);


where ``a`` and ``b`` are variables with unknown values. It is still possible to describe the result of the ``max`` function in terms of ``a`` and ``b``. The following expression describes the value of ``r``::

    ite (b > a) b a

where ``ite`` is the "if-then-else" mathematical function, which based on the value of the first argument returns either the second or third. One subtlety of constructing this expression, however, is the treatment of conditionals in the original program. For any concrete values of ``a`` and ``b``, only one branch of the ``if`` statement will execute. During symbolic execution, on the other hand, it is necessary to execute both branches, track two different program states (each composed of symbolic values), and then merge those states after executing the ``if`` statement. This merging process takes into account the original branch condition and introduces the ``ite`` expression.

A symbolic execution system, then, is very similar to an interpreter that has a different notion of what constitutes a value and executes all paths through the program instead of just one. Therefore, the execution process is similar to that of a normal interpreter, and the process of generating a model for a piece of code is similar to building a test harness for that same code.

More specifically, the setup process for a test harness typically takes the following form:

* Initialize or allocate any resources needed by the code. For Java and LLVM code, this typically means allocating memory and setting the initial values of variables.

* Execute the code.

* Check the desired properties of the system state after the code completes.

Accordingly, three pieces of information are particularly relevant to the symbolic execution process, and are therefore needed as input to the symbolic execution system:

* The initial (potentially symbolic) state of the system.

* The code to execute.

* The final state of the system, and which parts of it are relevant to the properties being tested.

In the following sections, we describe how the Java and LLVM analysis primitives work in the context of these key concepts. We start with the simplest situation, in which the structure of the initial and final states can be directly inferred, and move on to more complex cases that require more information from the user.

Symbolic Termination
--------------------

Above we described the process of executing multiple branches and merging the results when encountering a conditional statement in the program. When a program contains loops, the branch that chooses to continue or terminate a loop could go either way. Therefore, without a bit more information, the most obvious implementation of symbolic execution would never terminate when executing programs that contain loops.

The solution to this problem is to analyze the branch condition whenever considering multiple branches. If the condition for one branch can never be true in the context of the current symbolic state, there is no reason to execute that branch, and skipping it can make it possible for symbolic execution to terminate.

Directly comparing the branch condition to a constant can sometimes be enough to ensure termination. For example, in simple, bounded loops like the following, comparison with a constant is sufficient.

.. code-block:: C

    for (int i = 0; i < 10; i++) {
        // do something
    }

In this case, the value of ``i`` is always concrete, and will eventually reach the value ``10``, at which point the branch corresponding to continuing the loop will be infeasible.

As a more complex example, consider the following function:

.. code-block:: C

    uint8_t f(uint8_t i) {
      int done = 0;
      while (!done) {
        if (i % 8 == 0) done = 1;
        i += 5;
      }
      return i;
    }

The loop in this function can only be determined to symbolically terminate if the analysis takes into account algebraic rules about common multiples. Similarly, it can be difficult to prove that a base case is eventually reached for all inputs to a recursive program.

In this particular case, however, the code *is* guaranteed to terminate after a fixed number of iterations (where the number of possible iterations is a function of the number of bits in the integers being used). To show that the last iteration is in fact the last possible one, it’s necessary to do more than just compare the branch condition with a constant. Instead, we can use the same proof tools that we use to ultimately analyze the generated models to, early in the process, prove that certain branch conditions can never be true (i.e., are unsatisfiable).

Normally, most of the Java and LLVM analysis commands simply compare branch conditions to the constant ``True`` or ``False`` to determine whether a branch may be feasible. However, each form of analysis allows branch satisfiability checking to be turned on if needed, in which case functions like ``f`` above will terminate.

Next, we examine the details of the specific commands available to analyze JVM and LLVM programs.

Loading Code
------------

The first step in analyzing any code is to load it into the system.

To load LLVM code, simply provide the location of a valid bitcode file to the :saw:ref:`llvm_load_module` function.

.. saw:command:: llvm_load_module
  :type: String -> TopLevel LLVMModule

  Load LLVM bitcode into SAW.

The resulting :saw:ref:`LLVMModule` can be passed into the various functions described below to perform analysis of specific LLVM functions.


The LLVM bitcode parser should generally work with LLVM versions between 3.5 and 9.0, though it may be incomplete for some versions. Debug metadata has changed somewhat throughout that version range, so is the most likely case of incompleteness. We aim to support every version after 3.5, however, so report any parsing failures as `on GitHub <https://github.com/GaloisInc/saw-script/issues>`_.

Loading Java code is slightly more complex, because of the more structured nature of Java packages. First, when running ``saw``, two flags control where to look for classes. The ``-j`` flag takes the name of a JAR file as an argument and adds the contents of that file to the class database. The ``-c`` flag takes the name of a directory as an argument and adds all class files found in that directory (and its subdirectories) to the class database. By default, the current directory is included in the class path. However, the Java runtime and standard library (usually called ``rt.jar``) is generally required for any non-trivial Java code, and can be installed in a wide variety of different locations. Therefore, for most Java analysis, you must provide the ``-j`` argument or the ``SAW_JDK_JAR`` environment variable to specify where to find this file.

Once the class path is configured, you can pass the name of a class to the java_load_class function.

.. saw:command:: java_load_class
  :type: String -> TopLevel JavaClass

  Load a Java class from JVM bytecode.

     

The resulting :saw:ref:`JavaClass` can be passed into the various functions described below to perform analysis of specific Java methods.

Java class files from any JDK newer than version 6 should work. However, JDK version 9 and newer do not contain a JAR file containing the standard libraries, and therefore do not currently work with SAW. We are investigating the best way to resolve this issue.


Notes on Compiling Code for SAW
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



SAW will generally be able to load arbitrary LLVM bitcode and JVM bytecode files, but several guidelines can be help make verification easier or more likely to succeed. For generating LLVM with ``clang``, it can be helpful to:


* Turn on debugging symbols with ``-g`` so that SAW can find source locations of functions, names of variables, etc.

* Optimize with ``-O1`` so that the generated bitcode more closely matches the C/C++ source, making the results more comprehensible.

* Use ``-fno-threadsafe-statics`` to prevent ``clang`` from emitting unnecessary pthread code.

* Link all relevant bitcode with ``llvm-link`` (including, e.g., the C++ standard library when analyzing C++ code).
     

All SAW proofs include side conditions to rule out undefined behavior, and proofs will only succeed if all of these side conditions have been discharged. However the default SAW notion of undefined behavior is with respect to the semantics of LLVM, rather than C or C++. If you want to rule out undefined behavior according to the C or C++ standards, consider compiling your code with ``-fsanitize=undefined`` or one of the `related flags <https://clang.llvm.org/docs/UsersManual.html#controlling-code-generation>`_ to ``clang``.

Generally, you’ll also want to use ``-fsanitize-trap=undefined``, or one of the related flags, to cause the compiled code to use ``llvm.trap`` to indicate the presence of undefined behavior. Otherwise, the compiled code will call a separate function, such as ``__ubsan_handle_shift_out_of_bounds``, for each type of undefined behavior, and SAW currently does not have built in support for these functions (though you could manually create overrides for them in a verification script).

For Java, the only compilation flag that tends to be valuable is ``-g`` to retain information about the names of function arguments and local variables.


Notes on C++ Analysis
~~~~~~~~~~~~~~~~~~~~~



The distance between C++ code and LLVM is greater than between C and LLVM, so some additional considerations come into play when analyzing C++ code with SAW.

The first key issue is that the C++ standard library is large and complex, and tends to be widely used by C++ applications. To analyze most C++ code, it will be necessary to link your code with a version of the ``libc++`` `library <https://libcxx.llvm.org/docs/BuildingLibcxx.html>`_ compiled to LLVM bitcode. The ``wllvm`` `program <https://github.com/travitch/whole-program-llvm>`_ can be useful for this.

The C++ standard library includes a number of key global variables, and any code that touches them will require that they be initialized using :saw:ref:`crucible_alloc_global`.

Many C++ names are slightly awkward to deal with in SAW. They may be mangled relative to the text that appears in the C++ source code. SAW currently only understands the mangled names. The ``llvm-nm`` program can be used to show the list of symbols in an LLVM bitcode file, and the ``c++filt`` program can be used to demangle them, which can help in identifying the symbol you want to refer to. In addition, C++ names from namespaces can sometimes include quote marks in their LLVM encoding. For example::

    %"class.quux::Foo" = type { i32, i32 }

This can be mentioned in SAW by saying::

    llvm_type "%\"class.quux::Foo\""

Finally, there is no support for calling constructors in specifications, so you will need to construct objects piece-by-piece using, e.g., :saw:ref:`crucible_alloc` and :saw:ref:`crucible_points_to`.


Direct Extraction
-----------------

In the case of the ``max`` function described earlier, the relevant inputs and outputs are immediately apparent. The function takes two integer arguments, always uses both of them, and returns a single integer value, making no other changes to the program state.



In cases like this, a direct translation is possible, given only an identification of which code to execute. Two functions exist to handle such simple code. The first, for LLVM is the more stable of the two. A similar function exists for Java, but is more experimental.
     
The structure of these two extraction functions is essentially identical. The first argument describes where to look for code (in either a Java class or an LLVM module, loaded as described in the previous section). The second argument is the name of the method or function to extract.

When the extraction functions complete, they return a :saw:ref:`Term` corresponding to the value returned by the function or method as a function of its arguments.

.. saw:command:: crucible_llvm_extract
  :type: LLVMModule -> String -> TopLevel Term

  Extract a definition from an LLVM module into a :saw:ref:`Term`.

.. saw:command:: crucible_java_extract
  :type: JavaClass -> String -> TopLevel Term

  Extract a definition from a Java class into a :saw:ref:`Term`.

.. saw:command:: enable_experimental
  :type: TopLevel ()

  Enable unstable experimental features of SAW.

These functions currently work only for code that takes some fixed number of integral parameters, returns an integral result, and does not access any dynamically-allocated memory (although temporary memory allocated during execution is allowed).

Creating Symbolic Variables
---------------------------

The direct extraction process just discussed automatically introduces symbolic variables and then abstracts over them, yielding a SAWScript :saw:ref:`Term` that reflects the semantics of the original Java or LLVM code. For simple functions, this is often the most convenient interface. For more complex code, however, it can be necessary (or more natural) to specifically introduce fresh variables and indicate what portions of the program state they correspond to.



.. saw:command:: fresh_symbolic
  :type: String -> Type -> TopLevel Term

  Creates a fresh symbolic variable. The first argument is a name used for pretty-printing of terms and counter-examples. In many cases it makes sense for this to be the same as the name used within SAWScript, as in the following::

    x <- fresh_symbolic "x" ty;
  
  However, using the same name is not required.

  The second argument is the type of the fresh variable. Ultimately, this will be a SAWCore type; however, it is usually convenient to specify it using Cryptol syntax with the type quoting brackets ``{|`` and ``|}``. For example, creating a 32-bit integer, as might be used to represent a Java int or an LLVM i32, can be done as follows::

    x <- fresh_symbolic "x" {| [32] |};

  
Although symbolic execution works best on symbolic variables, which are “unbound” or “free”, most of the proof infrastructure within SAW uses variables that are bound by an enclosing lambda expression. Given a :saw:ref:`Term` with free symbolic variables, we can construct a lambda term that binds them in several ways.


.. saw:function:: abstract_symbolic
  :type: Term -> Term

Finds all symbolic variables in the Term and constructs a lambda expression binding each one, in some order. The result is a function of some number of arguments, one for each symbolic variable. It is the simplest but least flexible way to bind symbolic variables.


For example::

    sawscript> x <- fresh_symbolic "x" {| [8] |}
    sawscript> let t = {{ x + x }}
    sawscript> print_term t
    let { x@1 = Prelude.Vec 8 Prelude.Bool
        }
     in Cryptol.ecPlus x@1 (Cryptol.PArithSeqBool (Cryptol.TCNum 8))
          x
          x
    sawscript> let f = abstract_symbolic t
    sawscript> print_term f
    let { x@1 = Prelude.Vec 8 Prelude.Bool
        }
     in \(x : x@1) ->
          Cryptol.ecPlus x@1 (Cryptol.PArithSeqBool (Cryptol.TCNum 8)) x x


If there are multiple symbolic variables in the :saw:ref:`Term` passed to :saw:ref:`abstract_symbolic`, the ordering of parameters can be hard to predict. In some cases (such as when a proof is the immediate next step, and it’s expected to succeed) the order isn’t important. In others, it’s nice to have more control over the order.

.. saw:function:: lambda
  :type: Term -> Term -> Term

  The building block for controlled binding. It takes two terms: the one to transform, and the portion of the term to abstract over. Generally, the first :saw:ref:`Term` is one obtained from fresh_symbolic and the second is a :saw:ref:`Term` that would be passed to :saw:ref:`abstract_symbolic`::

    sawscript> let f = lambda x t
    sawscript> print_term f
    let { x@1 = Prelude.Vec 8 Prelude.Bool
        }
     in \(x : x@1) ->
          Cryptol.ecPlus x@1 (Cryptol.PArithSeqBool (Cryptol.TCNum 8)) x x


.. saw:function:: lambdas
  :type: [Term] -> Term -> Term

  Allows you to list the order in which symbolic variables should be bound.

Consider, for example, a :saw:ref:`Term` which adds two symbolic variables::

    sawscript> x1 <- fresh_symbolic "x1" {| [8] |}
    sawscript> x2 <- fresh_symbolic "x2" {| [8] |}
    sawscript> let t = {{ x1 + x2 }}
    sawscript> print_term t
    let { x@1 = Prelude.Vec 8 Prelude.Bool
        }
     in Cryptol.ecPlus x@1 (Cryptol.PArithSeqBool (Cryptol.TCNum 8))
          x1
          x2

  We can turn t into a function that takes x1 followed by x2::

    sawscript> let f1 = lambdas [x1, x2] t
    sawscript> print_term f1
    let { x@1 = Prelude.Vec 8 Prelude.Bool
        }
     in \(x1 : x@1) ->
          \(x2 : x@1) ->
            Cryptol.ecPlus x@1 (Cryptol.PArithSeqBool (Cryptol.TCNum 8)) x1
              x2

Or we can turn t into a function that takes x2 followed by x1::

    sawscript> let f1 = lambdas [x2, x1] t
    sawscript> print_term f1
    let { x@1 = Prelude.Vec 8 Prelude.Bool
        }
     in \(x2 : x@1) ->
          \(x1 : x@1) ->
            Cryptol.ecPlus x@1 (Cryptol.PArithSeqBool (Cryptol.TCNum 8)) x1
              x2

Specification-Based Verification
--------------------------------


The built-in functions described so far work by extracting models of code that can then be used for a variety of purposes, including proofs about the properties of the code.

When the goal is to prove equivalence between some LLVM or Java code and a specification, however, a more declarative approach is sometimes convenient. The following sections describe an approach that combines model extraction and verification with respect to a specification. A verified specification can then be used as input to future verifications, allowing the proof process to be decomposed.

Running a Verification
~~~~~~~~~~~~~~~~~~~~~~

Verification of LLVM is controlled by the crucible_llvm_verify command.

.. saw:command:: crucible_llvm_verify
  :type: LLVMModule -> String -> [CrucibleMethodSpec] -> Bool -> CrucibleSetup () -> ProofScript SatResult -> TopLevel CrucibleMethodSpec

  The first two arguments specify the module and function name to verify, as with :saw:ref:`llvm_verify`. The third argument specifies the list of already-verified specifications to use for compositional verification (described later; use ``[]`` for now). The fourth argument specifies whether to do path satisfiability checking, and the fifth gives the specification of the function to be verified. Finally, the last argument gives the proof script to use for verification. The result is a proved specification that can be used to simplify verification of functions that call this one.

A similar command for JVM programs is available if :saw:ref:`enable_experimental` has been run.

.. saw:command:: crucible_jvm_verify
  :type: JavaClass -> String -> [JVMMethodSpec] -> Bool -> JVMSetup () -> ProofScript SatResult -> TopLevel JVMMethodSpec
  
  See :saw:ref:`crucible_llvm_verify` for a description of the arguments.


Now we describe how to construct a value of type ``CrucibleSetup ()`` (or ``JVMSetup ()``).

Structure of a Specification
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A specifications for Crucible consists of three logical components:

* A specification of the initial state before execution of the function.

* A description of how to call the function within that state.

* A specification of the expected final value of the program state.

These three portions of the specification are written in sequence within a :saw:ref:`do` block of :saw:ref:`CrucibleSetup` (or :saw:ref:`JVMSetup`) type. The command :saw:ref:`crucible_execute_func` (or :saw:ref:`jvm_execute_func`) separates the specification of the initial state from the specification of the final state, and specifies the arguments to the function in terms of the initial state. Most of the commands available for state description will work either before or after :saw:ref:`crucible_execute_func`, though with slightly different meaning, as described below.

Creating Fresh Variables
~~~~~~~~~~~~~~~~~~~~~~~~

In any case where you want to prove a property of a function for an entire class of inputs (perhaps all inputs) rather than concrete values, the initial values of at least some elements of the program state must contain fresh variables. These are created in a specification with the :saw:ref:`crucible_fresh_var` and :saw:ref:`jvm_fresh_var` commands rather than :saw:ref:`fresh_symbolic`.

.. saw:command:: crucible_fresh_var
  :type: String -> LLVMType -> CrucibleSetup Term

.. saw:command:: jvm_fresh_var
  :type: String -> JavaType -> JVMSetup Term

The first parameter to both functions is a name, used only for presentation. It’s possible (though not recommended) to create multiple variables with the same name, but SAW will distinguish between them internally. The second parameter is the LLVM (or Java) type of the variable. The resulting :saw:ref:`Term` can be used in various subsequent commands.

LLVM types are built with this set of functions:

.. saw:function:: llvm_int
  :type: Int -> LLVMType
.. saw:function:: llvm_array
  :type: Int -> LLVMType -> LLVMType
.. saw:function:: llvm_struct
  :type: String -> LLVMType
.. saw:function:: llvm_float
  :type: LLVMType
.. saw:function:: llvm_double
  :type: LLVMType

Java types are built up using the following functions:


.. saw:function:: java_bool
  :type: JavaType
.. saw:function:: java_byte
  :type: JavaType
.. saw:function:: java_char
  :type: JavaType
.. saw:function:: java_short
  :type: JavaType
.. saw:function:: java_int
  :type: JavaType
.. saw:function:: java_long
  :type: JavaType
.. saw:function:: java_float
  :type: JavaType
.. saw:function:: java_double
  :type: JavaType
.. saw:function:: java_class
  :type: String -> JavaType
.. saw:function:: java_array
  :type: Int -> JavaType -> JavaType


     

Most of these types are straightforward mappings to the standard LLVM and Java types. The one key difference is that arrays must have a fixed, concrete size. Therefore, all analysis results are valid only under the assumption that any arrays have the specific size indicated, and may not hold for other sizes. The :saw:ref:`llvm_int` function also takes an :saw:ref:`Int` parameter indicating the variable’s bit width.



LLVM types can also be specified in LLVM syntax directly by using the :saw:ref:`llvm_type` function.


.. saw:function:: llvm_type
  :type: String -> LLVMType


For example, ``llvm_type "i32"`` yields the same result as ``llvm_int 32``.

The most common use for creating fresh variables is to state that a particular function should have the specified behaviour for arbitrary initial values of the variables in question. Sometimes, however, it can be useful to specify that a function returns (or stores, more about this later) an arbitrary value, without specifying what that value should be. To express such a pattern, you can also run :saw:ref:`crucible_fresh_var` from the post state (i.e., after :saw:ref:`crucible_execute_func`).

The SetupValue and JVMValue Types
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Many specifications require reasoning about both pure values and about the configuration of the heap. The :saw:ref:`SetupValue` type corresponds to values that can occur during symbolic execution, which includes both :saw:ref:`Term` values, pointers, and composite types consisting of either of these (both structures and arrays).

.. saw:type:: SetupValue
  Values that can occur during symbolic execution of LLVM code.

.. saw:type:: JVMValue
  Values that can occur during symbolic execution of JVM code.

The :saw:ref:`crucible_term` and :saw:ref:`jvm_term` functions create a :saw:ref:`SetupValue` or :saw:ref:`JVMValue` from a :saw:ref:`Term`:

.. saw:function:: crucible_term
  :type: Term -> SetupValue

.. saw:function:: jvm_term
  :type: Term -> JVMValue


Executing
~~~~~~~~~

Once the initial state has been configured, the :saw:ref:`crucible_execute_func` command specifies the parameters of the function being analyzed in terms of the state elements already configured.

.. saw:command:: crucible_execute_func
  :type: [SetupValue] -> CrucibleSetup ()


Return Values
~~~~~~~~~~~~~

To specify the value that should be returned by the function being verified use the :saw:ref:`crucible_return` or :saw:ref:`jvm_return` command.

.. saw:command:: crucible_return
  :type: SetupValue -> CrucibleSetup ()

.. saw:command:: jvm_return
  :type: JVMValue -> JVMSetup ()

A First Simple Example
~~~~~~~~~~~~~~~~~~~~~~



The commands introuduced so far are sufficient to verify simple programs that do not use pointers (or that use them only internally). Consider, for instance the C program that adds its two arguments together:

.. code-block:: C

    #include <stdint.h>
    uint32_t add(uint32_t x, uint32_t y) {
        return x + y;
    }

We can specify this function’s expected behavior as follows:

.. code-block:: SAWScript

    let add_setup = do {
        x <- crucible_fresh_var "x" (llvm_int 32);
        y <- crucible_fresh_var "y" (llvm_int 32);
        crucible_execute_func [crucible_term x, crucible_term y];
        crucible_return (crucible_term {{ x + y : [32] }});
    };

We can then compile the C file ``add.c`` into the bitcode file ``add.bc`` and verify it with ABC:

.. code-block:: SAWScript

    m <- llvm_load_module "add.bc";
    add_ms <- crucible_llvm_verify m "add" [] false add_setup abc;

Compositional Verification
~~~~~~~~~~~~~~~~~~~~~~~~~~

The primary advantage of the specification-based approach to verification is that it allows for compositional reasoning. That is, when proving properties of a given method or function, we can make use of properties we have already proved about its callees rather than analyzing them anew. This enables us to reason about much larger and more complex systems than otherwise possible.

The :saw:ref:`crucible_llvm_verify` and :saw:ref:`crucible_jvm_verify` functions return values of type :saw:ref:`CrucibleMethodSpec` and :saw:ref:`JVMMethodSpec`, respectively. These values are opaque objects that internally contain both the information provided in the associated :saw:ref:`JVMSetup` or :saw:ref:`CrucibleSetup` blocks and the results of the verification process.

Any of these ``MethodSpec`` objects can be passed in via the third argument of the ``..._verify`` functions. For any function or method specified by one of these parameters, the simulator will not follow calls to the associated target. Instead, it will perform the following steps:


* Check that all :saw:ref:`crucible_points_to` and :saw:ref:`crucible_precond` statements (or the corresponding JVM statements) in the specification are satisfied.

* Update the simulator state and optionally construct a return value as described in the specification.

More concretely, building on the previous example, say we have a doubling function written in terms of ``add``:

.. code-block:: C

    uint32_t dbl(uint32_t x) {
        return add(x, x);
    }

It has a similar specification to ``add``:

.. code-block:: SAWScript

    let dbl_setup = do {
        x <- crucible_fresh_var "x" (llvm_int 32);
        crucible_execute_func [crucible_term x];
        crucible_return (crucible_term {{ x + x : [32] }});
    };

And we can verify it using what we’ve already proved about ``add``:

.. code-block:: SAWScript

    crucible_llvm_verify m "dbl" [add_ms] false dbl_setup abc;

In this case, doing the verification compositionally doesn’t save computational effort, since the functions are so simple, but it illustrates the approach.
