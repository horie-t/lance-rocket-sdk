#include "defines.h"
#include "encoding.h"
#include "serial.h"

#define SERIAL_DEV_NUM 1

#define TinyLance_UART_0 ((volatile struct tinylance_uart *) 0x10013000)

/**
 * UART制御レジスタ構造体
 *
 * 割込みは、送信用FIFOのエントリ数がtxcntより小さい時、
 * または、受信用FIFOのエントリ数がrxcntより大きい時に起きる。
 */
struct tinylance_uart {
  volatile uint32_t txdata;	/* 送信データレジスタ(31: full FIFO満杯, 7-0: data)*/
  volatile uint32_t rxdata;	/* 受信データレジスタ(32: empty FIFO空, 7-0: data) */
  volatile uint32_t txctrl;	/* 送信制御レジスタ (18-16: txcnt TxFIFO watermark割り込みトリガしきい値, 
				   1: nstop ストップビット数(0が1つ、1が2つ), 0: txen アクティブ状態)*/
  volatile uint32_t rxctrl;	/* 受信制御レジスタ (18:16: rxcnt RxFIFO watermark割り込みトリガしきい値 
				   0: rxen アクティブ状態) */
  volatile uint32_t ie;		/* UART割り込み許可(1: 受信データ割り込み, 0: 送信データ割り込み */
  volatile uint32_t ip;		/* UART割り込み保留(1: 受信, 0: 送信) */
  volatile uint32_t div;	/* ボー・レート用除数(clock / baud_rate - 1 をセットする) */
};

/* TXDATA register */
#define UART_TXFULL		0x80000000
#define UART_RXEMPTY		0x80000000

/* TXCTRL register */
#define UART_TXEN               0x1
#define UART_TXNSTOP            0x2
#define UART_TXWM(x)            (((x) & 0xffff) << 16)
  
/* RXCTRL register */
#define UART_RXEN               0x1
#define UART_RXWM(x)            (((x) & 0xffff) << 16)
  
/* IP register */
#define UART_IP_TXWM            0x1
#define UART_IP_RXWM            0x2

#define UART_BAUT_RATE 9600

static struct {
  volatile struct tinylance_uart *uart;
} regs[SERIAL_DEV_NUM] = {
  {TinyLance_UART_0},
};

static int32_t received_val = -1;

int serial_init(int index)
{
  volatile struct tinylance_uart *uart = regs[index].uart;

  if (index != 0) {
    // 不正なindex
    return 1;
  }

  // GPIOの入出力を、UARTにつなげる
  *(uint32_t *)(GPIO_CTRL_ADDR + GPIO_IOF_SEL) &= ~IOF0_UART0_MASK;
  *(uint32_t *)(GPIO_CTRL_ADDR + GPIO_IOF_EN) |= IOF0_UART0_MASK;
  
  // PLICの優先度、有効化を設定(何故か設定しなくても動く)
  *(uint32_t *)(PLIC_CTRL_ADDR + PLIC_PRIORITY_OFFSET + SERIAL_PLIC_SOURCE * 4) = 1;
  *(uint32_t *)(PLIC_CTRL_ADDR + PLIC_ENABLE_OFFSET + SERIAL_PLIC_SOURCE / 32) |= 1 << (SERIAL_PLIC_SOURCE % 32);

  // UARTのボー・レート設定、有効化、割込みの閾値を設定
  uart->div = CPU_CLOCK / UART_BAUT_RATE - 1;
  uart->txctrl = UART_TXWM(1);
  uart->txctrl |= UART_TXEN;
  uart->rxctrl = UART_RXWM(0);	/* デフォルト値だけど一応セット */
  uart->rxctrl |= UART_RXEN;
  
  return 0;
}

int serial_is_send_enable(int index)
{
  volatile struct tinylance_uart *uart = regs[index].uart;
  return !(uart->txdata & UART_TXFULL);
}

int serial_send_byte(int index, unsigned char b)
{
  volatile struct tinylance_uart *uart = regs[index].uart;

  while (!serial_is_send_enable(index))
    ;

  uart->txdata = b;

  return 0;
}

int serial_is_recv_enable(int index)
{
  int32_t val;
  volatile struct tinylance_uart *uart = regs[index].uart;

  if (received_val > 0) {
    return 1;
  }

  val = (int32_t)uart->rxdata;
  if (val >= 0) {
    received_val = val;
    return 1;
  } else {
    return 0;
  }
}

unsigned char serial_recv_byte(int index)
{
  unsigned char c;

  while (!serial_is_recv_enable(index))
    ;

  c = received_val & 0xFF;
  received_val = -1;
  
  return c;
}

int serial_intr_is_send_enable(int index)
{
  volatile struct tinylance_uart *uart = regs[index].uart;
  return uart->ie & UART_IP_TXWM ? 1 : 0;
}

void serial_intr_send_enable(int index)
{
  volatile struct tinylance_uart *uart = regs[index].uart;
  uart->ie |= UART_IP_TXWM;
}

void serial_intr_send_disable(int index)
{
  volatile struct tinylance_uart *uart = regs[index].uart;
  uart->ie &= ~UART_IP_TXWM;
}

int serial_intr_is_recv_enable(int index)
{
  volatile struct tinylance_uart *uart = regs[index].uart;
  return uart->ie & UART_IP_RXWM ? 1 : 0;
}

void serial_intr_recv_enable(int index)
{
  volatile struct tinylance_uart *uart = regs[index].uart;
  uart->ie |= UART_IP_RXWM;
}

void serial_intr_recv_disable(int index)
{
  volatile struct tinylance_uart *uart = regs[index].uart;
  uart->ie &= ~UART_IP_RXWM;
}
