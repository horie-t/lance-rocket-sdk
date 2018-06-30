#include <stdio.h>

#include "platform.h"

#define LED3_OFFSET 14

int main()
{
  puts("LED On!\n");

  GPIO_REG(GPIO_OUTPUT_EN)  |= (1 << LED3_OFFSET) | (1 << LED5_OFFSET) | (1 << LED7_OFFSET) | (1 << LED9_OFFSET) | (1 << LED10_OFFSET) | (1 << LED12_OFFSET) | (1 << LED14_OFFSET);
  GPIO_REG(GPIO_OUTPUT_VAL) |= (1 << LED3_OFFSET) | (1 << LED5_OFFSET) | (1 << LED7_OFFSET) | (1 << LED9_OFFSET) | (1 << LED10_OFFSET) | (1 << LED12_OFFSET) | (1 << LED14_OFFSET);

  return 0;
}
