#include "encoding.h"

#define MTVEC_VECTORED     0x01

extern void vtrap_entry();

int mtvec_init(void)
{
  write_csr(mtvec, ((unsigned long)&vtrap_entry | MTVEC_VECTORED));

  return 0;
}
