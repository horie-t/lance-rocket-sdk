#ifndef _DEFINES_H_INCLUDED_
#define _DEFINES_H_INCLUDED_

#include <stdint.h>

#define NULL ((void*) 0)
#define SERIAL_DEFAULT_DEVICE 0

#define CPU_CLOCK 32500000	/* 32.5 MHz */

typedef uint32_t kz_thread_id_t;
typedef int (*kz_func_t)(int argc, char *argv[]);
typedef void (*kz_handler_t)(void);

typedef enum {
  MSGBOX_ID_CONSINPUT = 0,
  MSGBOX_ID_CONSOUTPUT,
  MSGBOX_ID_NUM
} kz_msgbox_id_t;

#endif
