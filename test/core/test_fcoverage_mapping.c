/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>

int main(int argc, char* argv[0]) {
  if (argc > 2) {
    printf("conditional!\n");
  }
  printf("Hello, world!\n");
  return 0;
}
