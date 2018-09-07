#include "defines.h"
#include "encoding.h"
#include "interrupt.h"
#include "kozos.h"
#include "lib.h"

kz_thread_id_t test09_1_id;
kz_thread_id_t test09_2_id;
kz_thread_id_t test09_3_id;

static int start_threads(int argc, char *argv[])
{
  test09_1_id = kz_run(test09_1_main, "test09_1", 1, 0x100, 0, NULL);
  test09_2_id = kz_run(test09_2_main, "test09_2", 2, 0x100, 0, NULL);
  test09_3_id = kz_run(test09_3_main, "test09_3", 3, 0x100, 0, NULL);

  kz_chpri(15);
  
  set_csr(mstatus, MSTATUS_MIE);

  while (1) {
    asm volatile ("wfi");
  }
  
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
  kz_start(start_threads, "idle", 0, 0x100, 0, NULL);
  /* ここには来ない */
  
  return 0;
}
