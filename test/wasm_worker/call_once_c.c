#include <stdio.h>
#include <threads.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/atomic.h>
#include <assert.h>

once_flag flag = ONCE_FLAG_INIT;
volatile int counter = 0;
volatile int finished_workers = 0;

void do_once() {
    emscripten_atomic_add_u32((void*)&counter, 1);
}

void worker_main() {
    call_once(&flag, do_once);
    emscripten_atomic_add_u32((void*)&finished_workers, 1);
}

int main() {
    emscripten_wasm_worker_t worker1 = emscripten_malloc_wasm_worker(1024 * 64);
    emscripten_wasm_worker_t worker2 = emscripten_malloc_wasm_worker(1024 * 64);

    emscripten_wasm_worker_post_function_v(worker1, worker_main);
    emscripten_wasm_worker_post_function_v(worker2, worker_main);

    while (emscripten_atomic_load_u32((void*)&finished_workers) < 2) {
    }

    printf("Counter: %d\n", counter);
    assert(counter == 1);
    printf("Success!\n");

    return 0;
}
