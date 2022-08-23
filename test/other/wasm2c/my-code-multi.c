#include <stdio.h>

// Note the prefixing (Z_liba, Z_libb) on all functions from the two libraries.

extern void Z_liba_wasmbox_init(void);
extern void Z_libb_wasmbox_init(void);

extern int (*Z_libaZ_twice)(int);
extern int (*Z_libbZ_thrice)(int);

int main() {
  puts("Initializing sandboxed unsafe libraries");
  Z_liba_wasmbox_init();
  Z_libb_wasmbox_init();
  printf("Calling twice on 21 returns %d\n", Z_libaZ_twice(21));
  printf("Calling thrice on 10 returns %d\n", Z_libbZ_thrice(10));
}
