/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten.h>
#include <features.h>

static int _main_argc;
static char** _main_argv;

int __main_argc_argv(int argc, char *argv[]);
weak void __wasm_call_ctors(void);

#ifdef __PIC__
int _emscripten_side_module_ctors(void);
#endif

weak int __main_void(void) {
  return __main_argc_argv(_main_argc, _main_argv);
}

EMSCRIPTEN_KEEPALIVE int _emscripten_start(int argc, char** argv) {
  if (__wasm_call_ctors) {
    __wasm_call_ctors();
  }
#ifdef __PIC__
  _emscripten_side_module_ctors();
#endif
  _main_argc = argc;
  _main_argv = argv;
  // Will either call user's __main_void or weak version above.
  return __main_void();
}
