/**
 * @license
 * Copyright 2023 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if !WASM_EXCEPTIONS

// These exception classes are only needed if we support exception catching,
// or if we use Emscripten-style longjmp (which uses JavaScript exceptions),
// or if we are building a main/side module where they might be needed by
// dynamically loaded code.
// Guarding this avoids emitting dead code (like EmscriptenEH) in builds that
// do not need them.
#if !DISABLE_EXCEPTION_CATCHING || SUPPORT_LONGJMP == 'emscripten' || MAIN_MODULE || SIDE_MODULE

// Base Emscripten EH error class
#if EXCEPTION_STACK_TRACES
class EmscriptenEH extends Error {}
#else
class EmscriptenEH {}
#endif

#if SUPPORT_LONGJMP == 'emscripten'
class EmscriptenSjLj extends EmscriptenEH {}
#endif

#if !DISABLE_EXCEPTION_CATCHING
class CppException extends EmscriptenEH {
  constructor(excPtr) {
#if EXCEPTION_STACK_TRACES
    super(excPtr);
#else
    super();
#endif
    this.excPtr = excPtr;
#if !DISABLE_EXCEPTION_CATCHING && EXCEPTION_STACK_TRACES
    const excInfo = getExceptionMessage(this);
    this.name = excInfo[0];
    this.message = excInfo[1];
#endif
  }
}
#endif

#endif // !DISABLE_EXCEPTION_CATCHING || SUPPORT_LONGJMP == 'emscripten' || MAIN_MODULE || SIDE_MODULE

#endif // !WASM_EXCEPTIONS
