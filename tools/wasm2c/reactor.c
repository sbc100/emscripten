#ifndef WASM2C_PREFIX
#error WASM2C_PREFIX must be defined
#endif

#define CONCAT2(A, B) A##B
#define CONCAT(A, B) CONCAT2(A, B)
#define ADD_PREFIX(TOKEN) CONCAT(WASM2C_PREFIX, TOKEN)

// TODO: optional prefixing
void ADD_PREFIX(_wasmbox_init)(void) {
  // Initialize wasm2c runtime.
  ADD_PREFIX(_init)();

  // Set up handling for a trap
  int trap_code;
  if ((trap_code = setjmp(wasm_rt_jmp_buf))) {
    printf("[wasm trap %d, halting]\n", trap_code);
    abort();
  }
  ADD_PREFIX(Z__initialize)();
}
