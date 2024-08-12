/*
 * Copyright 2024 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/param.h> // For roundup macro
#include <emscripten/heap.h>

ssize_t _emscripten_memory_grow(size_t new_size) {
  size_t old_size = __builtin_wasm_memory_size(0) * WASM_PAGE_SIZE;
  assert(new_size > old_size);
  size_t new_bytes = new_size - old_size;
  size_t new_pages = roundup(new_bytes, WASM_PAGE_SIZE) / WASM_PAGE_SIZE;
  return __builtin_wasm_memory_grow(0, new_pages);
}
