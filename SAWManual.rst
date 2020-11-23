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

Whether proving small lemmas (in the form of rewrite rules) or a top-level theorem, the process builds on the idea of a proof script that is run by one of the top level proof commands.
