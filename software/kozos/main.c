#include "defines.h"
#include "encoding.h"
#include "interrupt.h"
#include "kozos.h"
#include "lib.h"

/**
 * @brief システム・タスクとユーザ・タスクの起動
 */
static int start_threads(int argc, char *argv[])
{
  kz_run(consdrv_main, "consdrv", 1, 0x100, 0, NULL);
  kz_run(command_main, "command", 8, 0x100, 0, NULL);

  kz_chpri(15);
  
  set_csr(mstatus, MSTATUS_MIE);
  while (1) {
    asm volatile ("wfi");
  }
  
  return 0;
}

int main(void)
{
  /*
   * ソフトウェア割込み有効、タイマー割込み無効、外部割込み有効に設定
   */
  clear_csr(mstatus, MSTATUS_MIE);

  set_csr(mie, MIP_MSIP);
  clear_csr(mie, MIP_MTIP);
  set_csr(mie, MIP_MEIP);
  
  puts("kazos boot succeed!\n");

  /* OSの動作開始 */
  kz_start(start_threads, "idle", 0, 0x100, 0, NULL);
  /* ここには来ない */
  
  return 0;
}
