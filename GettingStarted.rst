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
concept of `undefined behavior <https://en.wikipedia.org/wiki/Undefined_behavior>`_ 
are assumed. It is not necessary to be
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

Troubleshooting / Installation alternatives
-------------------------------------------

If things don't succeed, the most likely cause is that you have a
newly-released version of LLVM.  SAW is dependent on LLVM's
`bitcode format<https://www.llvm.org/docs/BitCodeFormat.html>`, 
which often change between releases.  If you get an error
along these lines:

.. code-block::

   Are you sure you're using a supported version of LLVM/Clang?
   Check here: https://github.com/GaloisInc/llvm-pretty-bc-parser

you have a couple options:
  * install an earlier version of ``clang`` and configure your platform's 
    ``PATH`` to use it instead of the current version, or
  * use docker or vagrant to run ``saw`` and its tools in a virtual machine.
    The SAW VM configurations for docker and vagrant include known-good versions of all of SAW's
    dependencies. The SAW install page describes how to install SAW in
    a Docker container.

Using Vagrant to Install and Use SAW
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In some cases, it can be easiest to run the SAW tools in a virtual machine. Vagrant is a tool that manages the installation, configuration, starting and stopping of virtual machines with Linux guests. Here's how to run SAW in a Vagrant virtual machine:

- install `VirtualBox - instructions here <https://www.virtualbox.org/wiki/Downloads>`_
- install `Vagrant - instructions here <https://www.vagrantup.com/>`_
- cd to the ``examples`` directory unpacked from :download:`example files </examples.tar.gz>`, which includes a ``Vagrantfile``
- start and log in to the virtual machine with the SAW tools configured with these commands:


.. code-block::

  vagrant up       # launch the virtual machine
  vagrant ssh      # log in to your virtual machine
  cd examples
  make popcount.bc
  saw pop.saw      # should run to completion


- the first time you type ``vagrant up`` the system will download and configure SAW and its dependencies, so it will take a few minutes. Subsequent launches will be much faster.

- when you're done with a session, log out of the guest and cleanly shut down your virtual machine with the host command ``vagrant halt``

- editing files while logged in to a virtual machine can be inconvenient.  Vagrant guests have access to the host file system in the directory with the ``Vagrantfile``, which is located in the guest at ``/vagrant``, so it can be convenient to do your work in that directory, editing on your host, but running the SAW tools inside the virtual machine. In some cases you may have to install the "VirtualBox guest additions" to enable the shared ``vagrant`` folder.
