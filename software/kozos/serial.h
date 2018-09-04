#ifndef _SERIAL_H_INCLUDED_
#define _SERIAL_H_INCLUDED_

/**
 * @brief デバイスの初期化を実行します。
 */
int serial_init(int index);

/**
 * @breif 送信可能かどうかを返します。
 *
 * @return 送信可能ならば0以外、そうでなければ0を返します。
 */
int serial_is_send_enable(int index);

/**
 * @brief 1バイト送信します。
 *
 * @param b 送信するバイト
 */
int serial_send_byte(int index, unsigned char b);

/**
 * @brief 受信可能かどうかを返します。
 */
int serial_is_recv_enable(int index);

/**
 * @brief 1文字受信
 */
unsigned char serial_recv_byte(int index);

/**
 * @brief 送信割込みが有効か?
 */
int serial_intr_is_send_enable(int index);

/**
 * @brief 送信割込み有効化
 */
void serial_intr_send_enable(int index);

/**
 * @brief 送信割込み無効化
 */
void serial_intr_send_disable(int index);

/**
 * @brief 受信割込みが有効か?
 */
int serial_intr_is_recv_enable(int index);

/**
 * @brief 受信割込み有効化
 */
void serial_intr_recv_enable(int index);

/**
 * @brief 受信割込み無効化
 */
void serial_intr_recv_disable(int index);

#endif
