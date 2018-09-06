#include "defines.h"
#include "encoding.h"
#include "interrupt.h"
#include "kozos.h"
#include "lib.h"

static int start_threads(int argc, char *argv[])
{
  kz_run(test08_1_main, "command", 0x100, 0, NULL);
  return 0;
}

int main(void)
{
  puts("kozos boot start.\n");

  /*
   * ソフトウェア割込み有効、タイマー割込み有効、外部割込み無効に設定
   */
  clear_csr(mstatus, MSTATUS_MIE);

  set_csr(mie, MIP_MSIP);
  set_csr(mie, MIP_MTIP);
  clear_csr(mie, MIP_MEIP);
  
  puts("kazos boot succeed!\n");

  /* OSの動作開始 */
  kz_start(start_threads, "start", 0x100, 0, NULL);
  set_csr(mstatus, MSTATUS_MIE);
  /* ここには来ない */
  
  return 0;
}
