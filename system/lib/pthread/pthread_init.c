/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten/threading.h>

void __emscripten_init_main_thread_js(void* tb);
void _emscripten_thread_init(int, int, int);
int _emscripten_is_main_browser_thread();

static void *dummy_tsd[1] = { 0 };
weak_alias(dummy_tsd, __pthread_tsd_main);

static struct pthread __main_pthread;

pthread_t emscripten_main_browser_thread_id() {
  return &__main_pthread;
}

// See system/lib/README.md for static constructor ordering.
__attribute__((constructor(48)))
static void __init_main_thread(void) {
  __emscripten_init_main_thread_js(&__main_pthread);

  // The pthread struct has a field that points to itself - this is used as
  // a magic ID to detect whether the pthread_t structure is 'alive'.
  __main_pthread.self = &__main_pthread;
  __main_pthread.stack = (void*)emscripten_stack_get_base();
  __main_pthread.stack_size = emscripten_stack_get_base() - emscripten_stack_get_end();
  __main_pthread.detach_state = DT_JOINABLE;
  // pthread struct robust_list head should point to itself.
  __main_pthread.robust_list.head = &__main_pthread.robust_list.head;
  // Main thread ID is always 1.  It can't be 0 because musl assumes
  // tid is always non-zero.
  __main_pthread.tid = getpid();
  __main_pthread.locale = &libc.global_locale;
  // TODO(sbc): Implement circular list of threads
  //__main_pthread.next = __main_pthread.prev = &__main_pthread;
  __main_pthread.tsd = (void **)__pthread_tsd_main;
  _emscripten_thread_init(__main_pthread, _emscripten_is_main_browser_thread(), 1, 1);
}
