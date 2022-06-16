/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <features.h>

weak void __wasm_call_ctors(void);

void _initialize(void) {
  if (__wasm_call_ctors) {
    __wasm_call_ctors();
  }
}
