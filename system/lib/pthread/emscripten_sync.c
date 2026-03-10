/*
 * Copyright 2024 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <emscripten/atomic.h>
#include <emscripten/threading.h>
#include <emscripten/sync.h>

void emscripten_lock_init(emscripten_lock_t *lock) {
  emscripten_atomic_store_u32((void*)lock, EMSCRIPTEN_LOCK_T_STATIC_INITIALIZER);
}

void emscripten_lock_waitinf_acquire(emscripten_lock_t *lock) {
  for (;;) {
    uint32_t val = emscripten_atomic_cas_u32((void*)lock, 0, 1);
    if (val == 0) return;
    emscripten_futex_wait((void*)lock, val, INFINITY);
  }
}

bool emscripten_lock_wait_acquire(emscripten_lock_t *lock, int64_t maxWaitNanoseconds) {
  uint32_t val = emscripten_atomic_cas_u32((void*)lock, 0, 1);
  if (val == 0) return true;
  if (maxWaitNanoseconds <= 0) return false;

  int64_t waitEnd = (int64_t)(emscripten_performance_now() * 1e6) + maxWaitNanoseconds;
  while (maxWaitNanoseconds > 0) {
    emscripten_futex_wait((void*)lock, val, (double)maxWaitNanoseconds / 1e6);
    val = emscripten_atomic_cas_u32((void*)lock, 0, 1);
    if (val == 0) return true;
    maxWaitNanoseconds = waitEnd - (int64_t)(emscripten_performance_now() * 1e6);
  }
  return false;
}

bool emscripten_lock_busyspin_wait_acquire(emscripten_lock_t *lock, double maxWaitMilliseconds) {
  uint32_t val = emscripten_atomic_cas_u32((void*)lock, 0, 1);
  if (val == 0) return true;

  double t = emscripten_get_now();
  double waitEnd = t + maxWaitMilliseconds;
  while (t < waitEnd) {
    val = emscripten_atomic_cas_u32((void*)lock, 0, 1);
    if (val == 0) return true;
    t = emscripten_get_now();
  }
  return false;
}

void emscripten_lock_busyspin_waitinf_acquire(emscripten_lock_t *lock) {
  while (emscripten_atomic_cas_u32((void*)lock, 0, 1)) {
    // busy spin
  }
}

bool emscripten_lock_try_acquire(emscripten_lock_t *lock) {
  return emscripten_atomic_cas_u32((void*)lock, 0, 1) == 0;
}

void emscripten_lock_release(emscripten_lock_t *lock) {
  emscripten_atomic_store_u32((void*)lock, 0);
  emscripten_futex_wake((void*)lock, 1);
}

void emscripten_semaphore_init(emscripten_semaphore_t *sem, int num) {
  emscripten_atomic_store_u32((void*)sem, num);
}

int emscripten_semaphore_try_acquire(emscripten_semaphore_t *sem, int num) {
  uint32_t val = emscripten_atomic_load_u32((void*)sem);
  for (;;) {
    if (val < (uint32_t)num) return -1;
    uint32_t ret = emscripten_atomic_cas_u32((void*)sem, val, val - num);
    if (ret == val) return val - num;
    val = ret;
  }
}

int emscripten_semaphore_wait_acquire(emscripten_semaphore_t *sem, int num, int64_t maxWaitNanoseconds) {
  uint32_t val = emscripten_atomic_load_u32((void*)sem);
  int64_t waitEnd = (int64_t)(emscripten_performance_now() * 1e6) + maxWaitNanoseconds;
  for (;;) {
    while (val < (uint32_t)num) {
      if (maxWaitNanoseconds <= 0) return -1;
      emscripten_futex_wait((void*)sem, val, (double)maxWaitNanoseconds / 1e6);
      val = emscripten_atomic_load_u32((void*)sem);
      maxWaitNanoseconds = waitEnd - (int64_t)(emscripten_performance_now() * 1e6);
    }
    uint32_t ret = emscripten_atomic_cas_u32((void*)sem, val, val - num);
    if (ret == val) return val - num;
    val = ret;
  }
}

int emscripten_semaphore_waitinf_acquire(emscripten_semaphore_t *sem, int num) {
  uint32_t val = emscripten_atomic_load_u32((void*)sem);
  for (;;) {
    while (val < (uint32_t)num) {
      emscripten_futex_wait((void*)sem, val, INFINITY);
      val = emscripten_atomic_load_u32((void*)sem);
    }
    uint32_t ret = emscripten_atomic_cas_u32((void*)sem, val, val - num);
    if (ret == val) return val - num;
    val = ret;
  }
}

uint32_t emscripten_semaphore_release(emscripten_semaphore_t *sem, int num) {
  uint32_t ret = emscripten_atomic_add_u32((void*)sem, num);
  emscripten_futex_wake((void*)sem, num);
  return ret;
}

void emscripten_condvar_init(emscripten_condvar_t *condvar) {
  emscripten_atomic_store_u32((void*)condvar, EMSCRIPTEN_CONDVAR_T_STATIC_INITIALIZER);
}

void emscripten_condvar_waitinf(emscripten_condvar_t *condvar, emscripten_lock_t *lock) {
  uint32_t val = emscripten_atomic_load_u32((void*)condvar);
  emscripten_lock_release(lock);
  emscripten_futex_wait((void*)condvar, val, INFINITY);
  emscripten_lock_waitinf_acquire(lock);
}

bool emscripten_condvar_wait(emscripten_condvar_t *condvar, emscripten_lock_t *lock, int64_t maxWaitNanoseconds) {
  uint32_t val = emscripten_atomic_load_u32((void*)condvar);
  int64_t waitStart = (int64_t)(emscripten_performance_now() * 1e6);
  emscripten_lock_release(lock);
  emscripten_futex_wait((void*)condvar, val, (double)maxWaitNanoseconds / 1e6);
  int64_t waitEnd = (int64_t)(emscripten_performance_now() * 1e6);
  return emscripten_lock_wait_acquire(lock, maxWaitNanoseconds - (waitEnd - waitStart));
}

void emscripten_condvar_signal(emscripten_condvar_t *condvar, int64_t numWaitersToSignal) {
  emscripten_atomic_add_u32((void*)condvar, 1);
  emscripten_futex_wake((void*)condvar, (int)numWaitersToSignal);
}

#ifndef __EMSCRIPTEN_PTHREADS__
// In non-pthreads builds, we provide a thread-safe implementation of pthread_once
// so that C11 call_once and other once-init code works under WASM_WORKERS.
int __pthread_once(int *control, void (*init)(void)) {
  for (;;) {
    int val = emscripten_atomic_cas_u32(control, 0, 1);
    if (val == 0) {
      init();
      emscripten_atomic_store_u32(control, 2);
      emscripten_futex_wake(control, -1);
      return 0;
    }
    if (val == 2) return 0;
    // val is 1 (pending), wait for it to change
    emscripten_futex_wait(control, 1, INFINITY);
  }
}

// Emscripten's musl uses weak_alias for __pthread_once/pthread_once.
// Since we are not in musl here, we can just define it.
__attribute__((__weak__))
int pthread_once(int *control, void (*init)(void)) {
  return __pthread_once(control, init);
}
#endif
