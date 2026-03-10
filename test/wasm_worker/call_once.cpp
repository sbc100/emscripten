#include <iostream>
#include <mutex>
#include <emscripten/wasm_worker.h>
#include <emscripten/atomic.h>
#include <assert.h>

std::once_flag flag;
volatile int counter = 0;
volatile int finished_workers = 0;

void do_once() {
    emscripten_atomic_add_u32((void*)&counter, 1);
}

extern "C" {
void worker_main() {
    std::call_once(flag, do_once);
    emscripten_atomic_add_u32((void*)&finished_workers, 1);
}
}

int main() {
    if (!emscripten_atomics_is_lock_free(4)) {
        std::cout << "Atomics not supported, skipping test" << std::endl;
        return 0;
    }

    emscripten_wasm_worker_t worker1 = emscripten_malloc_wasm_worker(1024 * 64);
    emscripten_wasm_worker_t worker2 = emscripten_malloc_wasm_worker(1024 * 64);

    emscripten_wasm_worker_post_function_v(worker1, worker_main);
    emscripten_wasm_worker_post_function_v(worker2, worker_main);

    while (emscripten_atomic_load_u32((void*)&finished_workers) < 2) {
        // busy wait
    }

    std::cout << "Counter: " << counter << std::endl;
    if (counter == 1) {
        std::cout << "Success!" << std::endl;
    } else {
        std::cout << "Failure: counter is " << counter << std::endl;
        return 1;
    }

    return 0;
}
