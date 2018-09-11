#ifndef _KOZOS_H_INCLUDED_
#define _KOZOS_H_INCLUDED_

#include "defines.h"
#include "syscall.h"

/*****************************************
 * システム・コール関連
 *****************************************/
/**
 * @brief スレッド起動
 *
 * @param func スレッドで実行する関数
 * @param name スレッド名
 * @param priority 優先度。値が低い方が優先される。
 * @param stacksize スレッドのスタックのサイズ(単位はバイト)
 * @param argc funcへの引数の数(main関数と同様)
 * @param argv funcへの引数(main関数と同様)
 */
kz_thread_id_t kz_run(kz_func_t func, char *name, int priority, int stacksize, int argc, char *argv[]);

/**
 * @brief スレッド終了
 */
void kz_exit(void);

/**
 * @brief スレッドの実行権を他に渡す。POSIXのsched_yield()みたいなもの。
 */
int kz_wait();

/**
 * @brief スレッドを待機状態に遷移する。kz_wakeup()で呼ばれるまで実行可能状態にはならない。
 */
int kz_sleep();

/**
 * @brief 指定したスレッドを実行可能状態に遷移させる。
 */
int kz_wakeup(kz_thread_id_t id);

/**
 * @brief 自スレッドのIDを取得する。
 */
kz_thread_id_t kz_getid();

/**
 * @brief 自スレッドの優先度を変更する。
 */
int kz_chpri(int priority);

/**
 * @brief 動的メモリの獲得
 */
void *kz_kmalloc(int size);

/**
 * @brief メモリの解放
 */
int kz_kmfree(void *p);

/**
 * @brief メッセージの送信
 */
int kz_send(kz_msgbox_id_t id, int size, char *p);

/**
 * @brief メッセージの受信
 */
kz_thread_id_t kz_recv(kz_msgbox_id_t id, int *sizep, char **pp);

/**
 * @brief 割込みハンドラの登録
 */
int kz_setintr(softvec_type_t type, kz_handler_t handler);

/*****************************************
 * サービス・コール
 *****************************************/
int kx_wakeup(kz_thread_id_t id);

void *kx_kmalloc(int siez);

int kx_kmfree(void *p);

int kx_send(kz_msgbox_id_t id, int size, char *p);

/*****************************************
 * ライブラリ関数
 *****************************************/
/**
 * @brief 初期スレッドを起動して、OSの動作を開始する
 */
void kz_start(kz_func_t func, char *name, int priority, int statcksize, int argc, char *argv[]);

/**
 * @brief 致命的エラー発生時に呼び出す。
 */
void kz_sysdown(void);

/**
 * @brief システム・コールを実行する
 */
void kz_syscall(kz_syscall_type_t type, kz_syscall_param_t *param);

void kz_srvcall(kz_syscall_type_t type, kz_syscall_param_t *param);

/*****************************************
 * システム・タスク
 *****************************************/
int consdrv_main(int argc, char *argv[]);

/*****************************************
 * ユーザ・タスク
 *****************************************/
int command_main(int argc, char *argv[]);

#endif
