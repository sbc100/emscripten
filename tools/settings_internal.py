# Copyright 2019 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

#
# Settings in this file work exactly like those in settings.py but are not
# set-able from the command line and therefore are not part of the public
# ABI.  This means that these settings are an internal detail of the toolchain
# and can be added/removed/renamed without fear of breaking out users.
#

# List of symbols exported from compiled code
# These are raw symbol names and are not mangled to include the leading
# underscore.
WASM_EXPORTS = []

# Similar to above but only includes the functions symbols.
WASM_FUNCTION_EXPORTS = []

# An array of all symbols exported from all the side modules specified on the
# command line.
# These are raw symbol names and are not mangled to include the leading
# underscore.
SIDE_MODULE_EXPORTS = []

# All symbols imported by side modules.  These are symbols that the main
# module (or other side modules) will need to provide.
SIDE_MODULE_IMPORTS = []

# Like EXPORTED_FUNCTIONS, but will not error if symbol is missing
EXPORT_IF_DEFINED = ['__start_em_asm', '__stop_em_asm']

# Like EXPORTED_FUNCTIONS, but symbol is required to exist in native code.
# This means wasm-ld will fail if these symbols are missing.
REQUIRED_EXPORTS = []

# stores the base name of the output file (-o TARGET_BASENAME.js)
TARGET_BASENAME = ''

# stores the base name (with extension) of the output JS file
TARGET_JS_NAME = ''

# Indicates that the syscalls (which we see statically) indicate that they need
# full filesystem support. Otherwise, when just a small subset are used, we can
# get away without including the full filesystem - in particular, if open() is
# never used, then we don't actually need to support operations on streams.
SYSCALLS_REQUIRE_FILESYSTEM = 1

# A list of feature flags to pass to each binaryen invocation (like wasm-opt,
# etc.). This is received from wasm-emscripten-finalize, which reads it from
# the features section.
BINARYEN_FEATURES = []

# Whether EMCC_AUTODEBUG is on, which automatically instruments code for
# runtime logging that can help in debugging.
AUTODEBUG = 0

# Whether we should use binaryen's wasm2js to convert our wasm to JS. Set when
# wasm backend is in use with WASM=0 (to enable non-wasm output, we compile to
# wasm normally, then compile that to JS).
WASM2JS = 0

# Whether we should link in the runtime for ubsan.
# 0 means do not link ubsan, 1 means link minimal ubsan runtime.
# This is not meant to be used with `-s`. Instead, to use ubsan, use clang flag
# -fsanitize=undefined. To use minimal runtime, also pass
# `-fsanitize-minimal-runtime`.
UBSAN_RUNTIME = 0

# Whether we should link in LSan's runtime library. This is intended to be used
# by -fsanitize=leak instead of used directly.
USE_LSAN = 0

# Whether we should link in ASan's runtime library. This is intended to be used
# by -fsanitize=leak instead of used directly.
USE_ASAN = 0

# Whether we should load the WASM source map at runtime.
# This is enabled automatically when using -g4 with sanitizers.
LOAD_SOURCE_MAP = 0

# Whether embind has been enabled.
EMBIND = 0

# Whether the main() function reads the argc/argv parameters.
MAIN_READS_PARAMS = 1

# Name of the file containing the Fetch *.fetch.js, if relevant
FETCH_WORKER_FILE = ''

WASI_MODULE_NAME = "wasi_snapshot_preview1"

# List of JS libraries explictly linked against.  This includes JS system
# libraries (specified via -lfoo or -lfoo.js) in addition to user libraries
# passed via `--js-library`.  It does not include implicitly linked libraries
# added by the JS compiler.
JS_LIBRARIES = []

# This will contain the emscripten version. This can be useful in combination
# with RETAIN_COMPILER_SETTINGS
EMSCRIPTEN_VERSION = ''

# Will be set to 0 if -fno-rtti is used on the command line.
USE_RTTI = 1

# This will contain the optimization level (-Ox).
OPT_LEVEL = 0

# This will contain the debug level (-gx).
DEBUG_LEVEL = 0

# This will contain the shrink level (1 or 2 for -Os or -Oz, or just 0).
SHRINK_LEVEL = 0

# Whether or not to emit the name section in the final wasm binaryen.
EMIT_NAME_SECTION = 0

# Whether we are emitting a symbol map.
EMIT_SYMBOL_MAP = 0

# List of function explicitly exported by user on the command line.
USER_EXPORTED_FUNCTIONS = []

# name of the file containing wasm binary, if relevant
WASM_BINARY_FILE = ''

# name of the file containing the pthread *.worker.js, if relevant
PTHREAD_WORKER_FILE = ''

# Base URL the source mapfile, if relevant
SOURCE_MAP_BASE = ''

MEM_INIT_IN_WASM = 0

# If set to 1, src/base64Utils.js will be included in the bundle.
# This is set internally when needed (SINGLE_FILE)
SUPPORT_BASE64_EMBEDDING = 0

# the possible environments the code may run in.
ENVIRONMENT_MAY_BE_WEB = 1
ENVIRONMENT_MAY_BE_WORKER = 1
ENVIRONMENT_MAY_BE_NODE = 1
ENVIRONMENT_MAY_BE_SHELL = 1
ENVIRONMENT_MAY_BE_WEBVIEW = 1

# Whether to minify import and export names in the minify_wasm_js stage.
# Currently always off for MEMORY64.
MINIFY_WASM_IMPORTS_AND_EXPORTS = 0

# Whether to minify imported module names.
MINIFY_WASM_IMPORTED_MODULES = 0

# Whether to minify functions exported from Asm.js/Wasm module.
MINIFY_ASMJS_EXPORT_NAMES = 1

# Internal: represents a browser version that is not supported at all.
TARGET_NOT_SUPPORTED = 0x7FFFFFFF

# Wasm backend symbols that are considered system symbols and don't
# have the normal C symbol name mangled applied (== prefix with an underscore)
# (Also implicily on this list is any function that starts with string "dynCall_")
WASM_SYSTEM_EXPORTS = ['stackAlloc', 'stackSave', 'stackRestore']

# Internal: value of -flto argument (either full or thin)
LTO = 0

# Whether we may be accessing the address 2GB or higher. If so then we need
# to be using unsigned pointers in JS.
CAN_ADDRESS_2GB = 0

# Whether to emit DWARF in a separate wasm file on the side (this is not called
# "split" because there is already a DWARF concept by that name).
# When DWARF is on the side, the main file has no DWARF info, while the side
# file, ending in .debug.wasm, has the same wasm binary + all the debug
# sections.
# This has no effect if DWARF is not being emitted.
SEPARATE_DWARF = 0

# New WebAssembly exception handling (experimental)
EXCEPTION_HANDLING = 0

# Used internally when running the JS compiler simply to generate list of all
# JS symbols. This is used by LLD_REPORT_UNDEFINED to generate a list of all
# JS library symbols.
ONLY_CALC_JS_SYMBOLS = 0

# Set to true if the program has a main function.  By default this is
# enabled, but if `--no-entry` is passed, or if `_main` is not part of
# EXPORTED_FUNCTIONS then this gets set to 0.
EXPECT_MAIN = 1

# Provide and export a .ready() Promise. This is currently used by default with
# MODULARIZE, and returned from the factory function.
EXPORT_READY_PROMISE = 1

# struct_info that is either generated or cached
STRUCT_INFO = ''

# If true, building against Emscripten's wasm heap memory profiler.
MEMORYPROFILER = 0

GENERATE_SOURCE_MAP = 0

# Memory layout.  These are only used/set in RELOCATABLE builds.  Otherwise
# memory layout is fixed in the wasm binary at link time.
STACK_BASE = 0
STACK_MAX = 0
HEAP_BASE = 0

# Used internally. set when there is a main() function.
# Also set when in a linkable module, as the main() function might
# arrive from a dynamically-linked library, and not necessarily
# the current compilation unit.
# Also set for STANDALONE_WASM since the _start function is needed to call
# static ctors, even if there is no user main.
HAS_MAIN = 0

# Set to true if we are linking as C++ and including C++ stdlibs
LINK_AS_CXX = 0

# Set when some minimum browser version triggers doesn't support the
# minimum set of ES6 featurs.  This triggers transpilation to ES5
# using closure compiler.
TRANSPILE_TO_ES5 = 0
