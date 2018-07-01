#include <stdio.h>

#include "platform.h"

int main()
{
  puts("Seg7LED Display!\n");

  SEG7_REG(SEG7_VAL) = 0xCAFEBABE;

  return 0;
}
