/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten.h>

void waka(int x, int y, int z) {
  EM_ASM({
    out('received ' + [$0, $1, $2] + '.');
  }, x, y, z);
}

int main() {
  EM_ASM({
    var check = function(cond) { if (!cond) abort("check failed") };
#if EXPORTED
    // test for additional things being exported
    check(Module['addFunction']);
    check(Module['lengthBytesUTF8']);
    Module['setTempRet0'](42);
    check(Module['getTempRet0']() == 42);
    // the main test here
    Module['dynCall']('viii', $0, [1, 4, 9]);
#else
    // If 'ASSERTIONS' is enabled, these properties all exist, but with
    // stubs that show a useful error if called. So it is only meaningful
    // to check they don't exist when assertions are disabled.
    if (!ASSERTIONS) {
      check(!Module['addFunction']);
      check(!Module['lengthBytesUTF8']);
      check(!Module['setTempRet0']);
      check(!Module['getTempRet0']);
      check(!Module['dynCall']);
    }
    dynCall('viii', $0, [1, 4, 9]);
#endif
  }, &waka);
}

