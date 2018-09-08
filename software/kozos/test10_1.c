#include "defines.h"
#include "kozos.h"
#include "lib.h"

int test10_1_main(int argc, char *argv[])
{
  char *p1, *p2;
  int i, j;

  puts("test10_1 started.\n");

  for (i = 4; i < 56; i += 4) {
    p1 = kz_kmalloc(i);
    p2 = kz_kmalloc(i);

    for (j = 0; j < i - 1; j++) {
      p1[j] = 'a';
      p2[j] = 'b';
    }
    p1[j] = '\0';
    p2[j] = '\0';

    putxval((unsigned long)p1, 8); puts(" "); puts(p1); puts("\n");
    putxval((unsigned long)p2, 8); puts(" "); puts(p2); puts("\n");

    kz_kmfree(p1);
    kz_kmfree(p2);
  }

  puts("test10_1 exit\n");

  return 0;
}
