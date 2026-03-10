#include <stdio.h>
#include <emscripten/sync.h>
#include <emscripten/threading.h>
#include <emscripten/atomic.h>
#include <assert.h>

#if defined(__EMSCRIPTEN_PTHREADS__)
#include <pthread.h>
#elif defined(__EMSCRIPTEN_WASM_WORKERS__)
#include <emscripten/wasm_worker.h>
#endif

emscripten_lock_t lock = EMSCRIPTEN_LOCK_T_STATIC_INITIALIZER;
volatile int counter = 0;

void worker_func() {
    for (int i = 0; i < 1000; ++i) {
        emscripten_lock_waitinf_acquire(&lock);
        counter++;
        emscripten_lock_release(&lock);
    }
}

#if defined(__EMSCRIPTEN_PTHREADS__)
void* pthread_worker(void* arg) {
    worker_func();
    return NULL;
}
#elif defined(__EMSCRIPTEN_WASM_WORKERS__)
void wasm_worker_func() {
    worker_func();
}
#endif

int main() {
    const int num_threads = 4;
    const int expected_counter = num_threads * 1000;

#if defined(__EMSCRIPTEN_PTHREADS__)
    pthread_t threads[num_threads];
    for (int i = 0; i < num_threads; ++i) {
        pthread_create(&threads[i], NULL, pthread_worker, NULL);
    }
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
    }
#elif defined(__EMSCRIPTEN_WASM_WORKERS__)
    for (int i = 0; i < num_threads; ++i) {
        emscripten_wasm_worker_t worker = emscripten_malloc_wasm_worker(1024 * 64);
        emscripten_wasm_worker_post_function_v(worker, wasm_worker_func);
    }
    // Wait for workers to finish (using atomics as in previous test)
    // For simplicity, let's just use a simple atomic wait.
    // In this test environment, we'll just busy wait.
    // (In a real test we'd use a better way)
    while (counter < expected_counter) {
        // busy wait
    }
#endif

    printf("Counter: %d\n", counter);
    assert(counter == expected_counter);
    printf("Success!\n");

    return 0;
}
