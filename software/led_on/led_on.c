#include <stdio.h>

#include "platform.h"

#define LED3_OFFSET 14

int main()
{
  puts("LED3 On!\n");

  GPIO_REG(GPIO_OUTPUT_EN)  |= 1 << LED3_OFFSET;
  GPIO_REG(GPIO_OUTPUT_VAL) |= 1 << LED3_OFFSET;

  return 0;
}
