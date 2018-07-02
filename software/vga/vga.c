#include <stdio.h>

#include "platform.h"

#define VRAM ((volatile unsigned int *) VGA_MMAP_ADDR)

int main()
{
  int hexspeak = 0xACCE55ED;
  int n;

  puts("VGA VRAM!\n");

  VRAM[0] = hexspeak;
  n = VRAM[0];

  SEG7_REG(SEG7_VAL) = n;

  return 0;
}
