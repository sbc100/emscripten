#include <stdio.h>
#include <assert.h>

void side_print_foo();
int side_get_foo();
void* side_get_foo_addr();

int side_get_bar();
void* side_get_bar_addr();

__attribute__((weak)) int foo() {
  return 42;
}

__attribute__((weak)) int bar = 43;

int main(int argc, char const *argv[]) {
  printf("main foo() -> %d\n", foo());
  side_print_foo();

  // Check that addresses match
  assert(&foo == side_get_foo_addr());
  assert(&bar == side_get_bar_addr());

  // Check that main-moduled defined symbols take precedence.
  assert(bar == 43);
  assert(side_get_bar() == 43);

  assert(foo() == 42);
  assert(side_get_foo() == 42);

  return 0;
}
