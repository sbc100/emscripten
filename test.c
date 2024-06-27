#include <stdbool.h>
#include <stdio.h>

int main() {
  printf("%ld %ld\n", sizeof(bool), _Alignof(bool));
  return 0;
}
