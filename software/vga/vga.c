#include <stdio.h>

#include "platform.h"

#define VRAM ((volatile unsigned char *) VGA_MMAP_ADDR)
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480

void drawRect(int x, int y, int width, int height, char color)
{
  int px, py;

  for (px = x; px < x + width; px++) {
    VRAM[ y               * SCREEN_WIDTH + px] = color;
    VRAM[(y + height - 1) * SCREEN_WIDTH + px] = color;
  }

  for (py = y; py < y + height; py++) {
    VRAM[py * SCREEN_WIDTH + x            ] = color;
    VRAM[py * SCREEN_WIDTH + x + width - 1] = color;
  }
}


int main()
{
  puts("VGA VRAM!\n");

  drawRect(  0,   0, 640, 480, 0xFF);
  drawRect(100,  20, 320, 240, 0xE0);
  drawRect(200,  70, 100, 100, 0x1C);
  drawRect(360, 140, 200, 150, 0x03);

  return 0;
}
