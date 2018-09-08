#include "defines.h"
#include "kozos.h"
#include "lib.h"

int test09_1_main(int argc, char *argv[])
{
  puts("test09_1 started.\n");

  puts("test09_1 sleep in.\n");
  kz_sleep();
  puts("test09_1 sleep out.\n");

  puts("test09_1 chpri in.\n");
  kz_chpri(3);
  puts("test09_1 chpri out.\n");

  puts("test09_1 wait in.\n");
  kz_wait();
  puts("test09_1 wait out.\n");

  puts("test09_1 trap in.\n");
  asm volatile ("li t6, 0x7fffffff");
  asm volatile ("lw t6, 0(t6)");
  puts("test09_1 trap out.\n");
  
  puts("test09_1 exit.\n");

  return 0;
}
