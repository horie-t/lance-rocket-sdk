// See LICENSE.SiFive for license details.

#ifndef RISCV_CSR_ENCODING_H
#define RISCV_CSR_ENCODING_H

#define MSTATUS_MIE         0x00000008
#define MSTATUS_MPIE        0x00000080
#define MSTATUS_MPP         0x00001800

#define IRQ_M_SOFT   3
#define IRQ_M_TIMER  7
#define IRQ_M_EXT    11

#define MIP_MSIP            (1 << IRQ_M_SOFT)
#define MIP_MTIP            (1 << IRQ_M_TIMER)
#define MIP_MEIP            (1 << IRQ_M_EXT)

#define CAUSE_USER_ECALL 0x8
#define CAUSE_SUPERVISOR_ECALL 0x9
#define CAUSE_MACHINE_ECALL 0xb

#define SERIAL_PLIC_SOURCE 3

#define PLIC_CTRL_ADDR 		0x0C000000
#define PLIC_PRIORITY_OFFSET	0x0000
#define PLIC_ENABLE_OFFSET	0x2000
#define PLIC_CLAIM_OFFSET       0x200004

#define GPIO_CTRL_ADDR  0x10012000
#define GPIO_IOF_EN     (0x38)
#define GPIO_IOF_SEL    (0x3C)
#define IOF0_UART0_MASK         0x00030000


#define read_csr(reg) ({ unsigned long __tmp; \
  asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
  __tmp; })

#define write_csr(reg, val) ({ \
  if (__builtin_constant_p(val) && (unsigned long)(val) < 32) \
    asm volatile ("csrw " #reg ", %0" :: "i"(val)); \
  else \
    asm volatile ("csrw " #reg ", %0" :: "r"(val)); })

#define swap_csr(reg, val) ({ unsigned long __tmp; \
  if (__builtin_constant_p(val) && (unsigned long)(val) < 32) \
    asm volatile ("csrrw %0, " #reg ", %1" : "=r"(__tmp) : "i"(val)); \
  else \
    asm volatile ("csrrw %0, " #reg ", %1" : "=r"(__tmp) : "r"(val)); \
  __tmp; })

#define set_csr(reg, bit) ({ unsigned long __tmp; \
  if (__builtin_constant_p(bit) && (unsigned long)(bit) < 32) \
    asm volatile ("csrrs %0, " #reg ", %1" : "=r"(__tmp) : "i"(bit)); \
  else \
    asm volatile ("csrrs %0, " #reg ", %1" : "=r"(__tmp) : "r"(bit)); \
  __tmp; })

#define clear_csr(reg, bit) ({ unsigned long __tmp; \
  if (__builtin_constant_p(bit) && (unsigned long)(bit) < 32) \
    asm volatile ("csrrc %0, " #reg ", %1" : "=r"(__tmp) : "i"(bit)); \
  else \
    asm volatile ("csrrc %0, " #reg ", %1" : "=r"(__tmp) : "r"(bit)); \
  __tmp; })
#endif
