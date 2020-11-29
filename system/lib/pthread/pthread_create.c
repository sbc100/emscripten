/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#define _GNU_SOURCE
#include "pthread_impl.h"
#include "assert.h"
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <threads.h>
#include <emscripten/emmalloc.h>

// See musl's pthread_create.c

extern int __cxa_thread_atexit(void (*)(void *), void *, void *);
extern int __pthread_create_js(struct pthread *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
extern void _emscripten_thread_init(int, int, int);
extern void __pthread_exit_run_handlers();
extern void __pthread_exit_done();
extern int8_t __dso_handle;

static void dummy_0()
{
}
weak_alias(dummy_0, __pthread_tsd_run_dtors);

void __run_cleanup_handlers(void* _unused) {
  pthread_t self = __pthread_self();
  while (self->cancelbuf) {
    void (*f)(void *) = self->cancelbuf->__f;
    void *x = self->cancelbuf->__x;
    self->cancelbuf = self->cancelbuf->__next;
    f(x);
  }
}

void __do_cleanup_push(struct __ptcb *cb) {
  struct pthread *self = __pthread_self();
  cb->__next = self->cancelbuf;
  self->cancelbuf = cb;
  static thread_local bool registered = false;
  if (!registered) {
    __cxa_thread_atexit(__run_cleanup_handlers, NULL, &__dso_handle);
    registered = true;
  }
}

void __do_cleanup_pop(struct __ptcb *cb) {
  __pthread_self()->cancelbuf = cb->__next;
}

static int tl_lock_count;
static int tl_lock_waiters;

volatile int __thread_list_lock;

void __tl_lock(void) {
  int tid = __pthread_self()->tid;
  int val = __thread_list_lock;
  if (val == tid) {
    tl_lock_count++;
    return;
  }
  while ((val = a_cas(&__thread_list_lock, 0, tid)))
    __wait(&__thread_list_lock, &tl_lock_waiters, val, 0);
}

void __tl_unlock(void) {
  if (tl_lock_count) {
    tl_lock_count--;
    return;
  }
  a_store(&__thread_list_lock, 0);
  if (tl_lock_waiters) __wake(&__thread_list_lock, 1, 0);
}

void _emscripten_thread_exit(void* result) {
  struct pthread *self = __pthread_self();
  assert(self);

  __tl_lock();

  self->next->prev = self->prev;
  self->prev->next = self->next;
  self->prev = self->next = self;

  __tl_unlock();

  self->canceldisable = PTHREAD_CANCEL_DISABLE;
  self->cancelasync = PTHREAD_CANCEL_DEFERRED;
  self->result = result;

  __pthread_exit_run_handlers();

  // Call into the musl function that runs destructors of all thread-specific data.
  __pthread_tsd_run_dtors();

  if (self == emscripten_main_browser_thread_id()) {
    // FIXME(sbc): When pthread_exit causes the entire application to exit
    // we should be returning zero (according to the man page for pthread_exit).
    exit((intptr_t)result);
    return;
  }

  // We have the call the buildin free here since lsan handling for this thread
  // gets shut down during __pthread_tsd_run_dtors.
  emscripten_builtin_free(self->tsd);
  self->tsd = NULL;

  // Mark the thread as no longer running.
  // When we publish this, the main thread is free to deallocate the thread object and we are done.
  if (self->detach_state == DT_DETACHED) {
    self->detach_state = DT_EXITED;
  } else {
    self->detach_state = DT_EXITING;
    // wake any threads that might be waiting for us to exit
    emscripten_futex_wake(&self->detach_state, INT_MAX);
  }

  // Not hosting a pthread anymore in this worker, reset the info structures to null.
  _emscripten_thread_init(0, 0, 0); // Unregister the thread block inside the wasm module.
  __pthread_exit_done();
}

// Mark as `no_sanitize("address"` since emscripten_pthread_exit destroys
// the current thread and runs its exit handlers.  Without this asan injects
// a call to __asan_handle_no_return before emscripten_unwind_to_js_event_loop
// which seem to cause a crash later down the line.
__attribute__((no_sanitize("address")))
_Noreturn void __pthread_exit(void* retval) {
  _emscripten_thread_exit(retval);
  emscripten_unwind_to_js_event_loop();
}

int __pthread_create(pthread_t *restrict res, const pthread_attr_t *restrict attrp, void *(*entry)(void *), void *restrict arg) {
  // Note on LSAN: lsan intercepts/wraps calls to pthread_create so any
  // allocation we we do here should be considered leaks.
  // See: lsan_interceptors.cpp.
  if (!res) {
    return EINVAL;
  }
  struct pthread *self = __pthread_self();

  // Allocate thread block (pthread_t structure).
  struct pthread *new = malloc(sizeof(struct pthread));
  // zero-initialize thread structure.
  memset(new, 0, sizeof(struct pthread));

  // The pthread struct has a field that points to itself - this is used as a
  // magic ID to detect whether the pthread_t structure is 'alive'.
  new->self = new;
  new->tid = (uintptr_t)new;

  // pthread struct robust_list head should point to itself.
  new->robust_list.head = &new->robust_list.head;

  new->locale = &libc.global_locale;

  // Allocate memory for thread-local storage and initialize it to zero.
  new->tsd = malloc(PTHREAD_KEYS_MAX * sizeof(void*));
  memset(new->tsd, 0, PTHREAD_KEYS_MAX * sizeof(void*));

  //printf("start __pthread_create: %p\n", self);
  int rtn = __pthread_create_js(new, attrp, entry, arg);
  if (rtn != 0)
    return rtn;

  __tl_lock();

  new->next = self->next;
  new->prev = self;
  new->next->prev = new;
  new->prev->next = new;

  __tl_unlock();

  *res = new;
  //printf("done __pthread_create self=%p next=%p prev=%p new=%p\n", self, self->next, self->prev, new);
  return 0;
}

weak_alias(__pthread_create, emscripten_builtin_pthread_create);
weak_alias(__pthread_create, pthread_create);
weak_alias(__pthread_exit, pthread_exit);
