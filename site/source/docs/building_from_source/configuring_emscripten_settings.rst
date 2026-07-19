.. _configuring-emscripten-settings:

==================================================================
Configuring Emscripten Settings when Manually Building from Source
==================================================================

Emscripten can be configured via a :ref:`compiler
configuration file (emscripten.conf) <compiler-configuration-file>`. These settings
include paths to the tools (LLVM, Clang, Binaryen, etc.) and the compiler's
temporary directory for intermediate build files.

This configuration file is optional.  By default, emscripten will search
for the tools it needs in the ``PATH``.

This article explains how to create and update the file when you are building
Emscripten :ref:`manually <installing-from-source>` from source.


Creating the compiler configuration file
========================================

A settings file may be used when running :ref:`emcc <emccdoc>` (or any of the
other Emscripten tools). You can run ``emcc`` with ``--generate-config``
in order to generate one in the default location.

1. Navigate to the directory where you cloned the Emscripten repository.
2. Enter the command:

  ::

    ./emcc --generate-config

  You should get a ``An Emscripten settings file has been generated at:``
  message, along with the contents of the config file.

When generating this file Emscripten will make its "best guess" at the correct
locations for tools based on the current ``PATH``.

In most cases it will be necessary to edit the generated file and modify
at least the ``LLVM_ROOT`` and ``BINARYEN_ROOT`` settings to point to the correct
locations for your local LLVM and Binaryen installations.


Locating the compiler configuration file (emscripten.conf)
==========================================================

The settings file (``emscripten.conf``) is created by default within the emscripten
directory (alongside ``emcc`` itself). In cases where the emscripten directory
is read-only the standard XDG user configuration path will be used:

  - On Linux and macOS this file is located at **~/.config/emscripten.conf**
    (The legacy location is **~/.emscripten**).

  - On Windows the file can be found at a path like
    **C:/Users/yourusername/.config/emscripten.conf** (The legacy location being
    **C:/Users/yourusername/.emscripten**).


Compiler configuration file-format
==================================

The configuration file uses a simple INI-style key-value format.

The file assigns values to a number of *variables* representing the main
tools used by Emscripten. For example, if your binaryen installation is in
**C:\\tools\\binaryen\\**, then the file might contain the line: ::

  BINARYEN_ROOT = C:\tools\binaryen

Note that values do not require quotes. Standard environment variables like
``$HOME`` can be used in paths. In addition, the special variable ``$CFGDIR``
can be used to reference the directory containing the configuration file itself,
allowing paths to be specified relative to the config file location.


Legacy format and updating to the new format
============================================

Historically, Emscripten used a Python-based configuration file named ``.emscripten``
which was evaluated as Python code (e.g. ``LLVM_ROOT = '/path/to/llvm'``).

While legacy ``.emscripten`` files are still supported, users are encouraged to update
to the new ``emscripten.conf`` format.

To update an existing legacy ``.emscripten`` file:

1. Rename ``.emscripten`` to ``emscripten.conf`` (or move it to ``~/.config/emscripten.conf``).
2. Remove any surrounding quotes (``'`` or ``"``) around path values.
3. Replace Python list syntax (e.g. ``NODE_JS = ['node']``) with a simple string value (``NODE_JS = node``).
4. Replace Python code functions/imports (such as ``os.path.join(...)``) with direct paths or ``$CFGDIR`` / environment variables.


Editing the compiler configuration file
=======================================

The compiler configuration file can be edited with the text editor of your
choice. If you're building manually from source, you are most likely to have to
update the variable ``LLVM_ROOT``:

#. Edit the variable ``LLVM_ROOT`` to point to the directory where you built the
   LLVM binaries, such as:

    ::

      LLVM_ROOT = /home/ubuntu/a-path/llvm/build/bin

After setting those paths, run ``emcc`` again. It should again perform the sanity checks to test the specified paths. There are further validation tests available at :ref:`verifying-the-emscripten-environment`.
