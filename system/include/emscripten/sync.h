/*
 * Copyright 2024 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// A simple mutex primitive based on atomics and futexes.
// This is available in both pthreads and wasm workers builds.
// It is safe to use on the main browser thread.
#define emscripten_lock_t volatile uint32_t
#define EMSCRIPTEN_LOCK_T_STATIC_INITIALIZER 0

void emscripten_lock_init(emscripten_lock_t *lock);
void emscripten_lock_waitinf_acquire(emscripten_lock_t *lock);
bool emscripten_lock_wait_acquire(emscripten_lock_t *lock, int64_t maxWaitNanoseconds);
bool emscripten_lock_busyspin_wait_acquire(emscripten_lock_t *lock, double maxWaitMilliseconds);
void emscripten_lock_busyspin_waitinf_acquire(emscripten_lock_t *lock);
bool emscripten_lock_try_acquire(emscripten_lock_t *lock);
void emscripten_lock_release(emscripten_lock_t *lock);

// A simple semaphore primitive based on atomics and futexes.
#define emscripten_semaphore_t volatile uint32_t
#define EMSCRIPTEN_SEMAPHORE_T_STATIC_INITIALIZER(num) ((int)(num))

void emscripten_semaphore_init(emscripten_semaphore_t *sem, int num);
int emscripten_semaphore_try_acquire(emscripten_semaphore_t *sem, int num);
int emscripten_semaphore_wait_acquire(emscripten_semaphore_t *sem, int num, int64_t maxWaitNanoseconds);
int emscripten_semaphore_waitinf_acquire(emscripten_semaphore_t *sem, int num);
uint32_t emscripten_semaphore_release(emscripten_semaphore_t *sem, int num);

// A simple condition variable primitive based on atomics and futexes.
#define emscripten_condvar_t volatile uint32_t
#define EMSCRIPTEN_CONDVAR_T_STATIC_INITIALIZER 0

void emscripten_condvar_init(emscripten_condvar_t *condvar);
void emscripten_condvar_waitinf(emscripten_condvar_t *condvar, emscripten_lock_t *lock);
bool emscripten_condvar_wait(emscripten_condvar_t *condvar, emscripten_lock_t *lock, int64_t maxWaitNanoseconds);
void emscripten_condvar_signal(emscripten_condvar_t *condvar, int64_t numWaitersToSignal);

#ifdef __cplusplus
}
#endif
