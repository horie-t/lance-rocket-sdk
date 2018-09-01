#include "defines.h"
#include "serial.h"
#include "lib.h"

int main(void)
{
  static char buf[32];

  puts("Hello, world!\n");

  while (1) {
    puts("> ");			/* プロンプト表示 */
    gets((unsigned char *)buf);	/* シリアルからのコマンド受信 */

    if (!strncmp(buf, "echo ", 5)) {
      puts(buf + 5);
      puts("\n");
    } else if (!strcmp(buf, "exit")) {
      break;
    } else {
      puts("unknown.\n");
    }
  }
  
  return 0;
}
