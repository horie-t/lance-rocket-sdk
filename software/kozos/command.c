#include "defines.h"
#include "kozos.h"
#include "consdrv.h"
#include "lib.h"

/**
 * @brief コンソール・ドライバの使用開始をコンソール・ドライバに依頼する
 */
static void send_use(int index)
{
  char *p;

  p = kz_kmalloc(3);
  p[0] = '0';
  p[1] = CONSDRV_CMD_USE;
  p[2] = '0' + index;
  kz_send(MSGBOX_ID_CONSOUTPUT, 3, p);
}

/**
 * @brief コンソールへの文字列出力をコンソール・ドライバに依頼する
 */
static void send_write(char *str)
{
  char *p;
  int len;
  len = strlen(str);
  p = kz_kmalloc(len + 2);
  p[0] = '0';
  p[1] = CONSDRV_CMD_WRITE;
  memcpy(&p[2], str, len);
  kz_send(MSGBOX_ID_CONSOUTPUT, len + 2, p);
}

int command_main(int argc, char *argv[])
{
  char *p;
  int size;

  send_use(SERIAL_DEFAULT_DEVICE);

  while (1) {
    send_write("command> ");
    
    kz_recv(MSGBOX_ID_CONSINPUT, &size, &p);
    p[size] = '\0';		/* 配列外をアクセスしてしまっているが... */

    if (!strncmp(p, "echo ", 5)) {
      send_write(p + 5);
      send_write("\n");
    } else {
      send_write("unknown.\n");
    }

    kz_kmfree(p);
  }

  return 0;
}
