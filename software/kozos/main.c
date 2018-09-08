#include "defines.h"
#include "encoding.h"
#include "interrupt.h"
#include "kozos.h"
#include "lib.h"

static int start_threads(int argc, char *argv[])
{
  kz_run(test11_1_main, "test11_1", 1, 0x100, 0, NULL);
  kz_run(test11_2_main, "test11_2", 2, 0x100, 0, NULL);

  kz_chpri(15);
  
  while (1) {
    asm volatile ("wfi");
  }
  
  return 0;
}

int main(void)
{
  /*
   * ソフトウェア割込み有効、タイマー割込み有効、外部割込み無効に設定
   */
  clear_csr(mstatus, MSTATUS_MIE);

  set_csr(mie, MIP_MSIP);
  set_csr(mie, MIP_MTIP);
  clear_csr(mie, MIP_MEIP);
  
  puts("kazos boot succeed!\n");

  /* OSの動作開始 */
  kz_start(start_threads, "idle", 0, 0x100, 0, NULL);
  set_csr(mstatus, MSTATUS_MIE);
  /* ここには来ない */
  
  return 0;
}
