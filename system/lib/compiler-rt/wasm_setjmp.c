struct __WasmLongjmpArgs {
  void *env;
  int val;
};

thread_local struct __WasmLongjmpArgs __wasm_longjmp_args;

// Wasm EH allows us to throw and catch multiple values, but that requires
// multivalue support in the toolchain, whch is not reliable at the time.
// TODO Consider switching to throwing two values at the same time later.
void __wasm_longjmp(void *env, int val) {
  __wasm_longjmp_args.env = env;
  __wasm_longjmp_args.val = val;
  __builtin_wasm_throw(1, &__wasm_longjmp_args);
}
