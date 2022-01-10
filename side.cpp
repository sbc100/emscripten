#include <iostream>       // std::cout
#include <thread>         // std::thread
#include <pthread.h>
#include <sstream>
#include <unistd.h> //sleep
#include <emscripten.h>

extern "C" {

void check_cookie();

void foo2() {
  printf("in thread\n");
  check_cookie();
}

void foo() {
  check_cookie();
  foo2();
}

void* bar(void*) {
  printf("in thread\n");
  return NULL;
}

EMSCRIPTEN_KEEPALIVE int side_main() {
  std::thread first(foo);

  //pthread_t t;
  //pthread_create(&t, NULL, foo, NULL);
  printf("joining thread...\n");
  first.join();
  //pthread_join(t, NULL);
  printf("done\n");
  return 0;
}

}
