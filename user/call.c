#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int g(int x) {
  return x+3;
}

int f(int x) {
  return g(x);
}

void main(void) {
  unsigned int i = 0x00646c72;
  printf("x=%d y=%d", 3);
	printf("\nH%x Wo%s", 57616, &i);
  printf("\n%d %d", f(8)+1, 13);
  printf("\nx=%d y=%d\n", 3);
  exit(0);
}
