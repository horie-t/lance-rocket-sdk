#include "defines.h"
#include "encoding.h"
#include "interrupt.h"
#include "serial.h"
#include "lib.h"

void handle_m_external_interrupt()
{
  int c;
  static char buf[32];
  static int len = 0;

  c = getc();

  if (c != '\n') {
    buf[len++] = c;
  } else {
    buf[len++] = '\0';
    if (!strncmp(buf, "echo ", 5)) {
      puts(buf + 5);
      puts("\n");
    } else {
      puts("unknown.\n");
    }
    puts("> ");			/* プロンプト表示 */
    len = 0;
  }
}

int main(void)
{
  clear_csr(mstatus, MSTATUS_MIE);

  serial_init(SERIAL_DEFAULT_DEVICE);
  puts("kazos boot succeed!\n");
  
  mtvec_init();
  serial_intr_recv_enable(SERIAL_DEFAULT_DEVICE);

  puts("> ");

  set_csr(mie, MIP_MEIP);
  set_csr(mstatus, MSTATUS_MIE);

  while (1) {
    asm volatile ("wfi");	/* 割込み待ち */
  }
  
  return 0;
}
