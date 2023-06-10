#include <assert.h>
#include <stdio.h>

#include <emscripten/threading.h>
#include <emscripten/em_js.h>

EM_JS(int, is_worker, (), {return ENVIRONMENT_IS_WORKER});

void* thread_main(void* arg) {
  assert(is_worker());
  assert(!emscripten_is_main_browser_thread());
  assert(!emscripten_is_main_runtime_thread());
  printf("thread done\n");
  return NULL;
}

int main() {
  printf("is_worker=%d is_main_browser=%d is_main_runtime=%d\n",
         is_worker(),
         emscripten_is_main_runtime_thread(),
         emscripten_is_main_browser_thread());
#ifdef PROXY_TO_PTHREAD
  assert(is_worker());
  assert(!emscripten_is_main_runtime_thread());
  assert(!emscripten_is_main_browser_thread());
#else
  assert(!is_worker());
  assert(emscripten_is_main_runtime_thread());
  assert(emscripten_is_main_browser_thread());
#endif

  pthread_t t;
  pthread_create(&t, NULL, thread_main, NULL);
  pthread_join(t, NULL);
  printf("main done\n");

  return 0;
}
