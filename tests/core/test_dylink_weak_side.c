#include <stdio.h>

extern int foo();
extern int bar;

__attribute__((weak)) int foo() {
  return 99;
}

__attribute__((weak)) int bar = 100;

void side_print_foo() {
  printf("side foo() -> %d\n", foo());
}

int side_get_foo() {
  return foo();
}

void* side_get_foo_addr() {
  return &foo;
}

int side_get_bar() {
  return bar;
}

void* side_get_bar_addr() {
  return &bar;
}
