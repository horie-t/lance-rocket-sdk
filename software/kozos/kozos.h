#ifndef _KOZOS_H_INCLUDED_
#define _KOZOS_H_INCLUDED_

#include "defines.h"
#include "syscall.h"

/*****************************************
 * システム・コール関連
 *****************************************/
/**
 * @brief スレッド起動
 */
kz_thread_id_t kz_run(kz_func_t func, char *name, int stacksize, int argc, char *argv[]);

/**
 * @brief スレッド終了
 */
void kz_exit(void);

/*****************************************
 * ライブラリ関数
 *****************************************/
/**
 * @brief 初期スレッドを起動して、OSの動作を開始する
 */
void kz_start(kz_func_t func, char *name, int statcksize, int argc, char *argv[]);

/**
 * @brief 致命的エラー発生時に呼び出す。
 */
void kz_sysdown(void);

/**
 * @brief システム・コールを実行する
 */
void kz_syscall(kz_syscall_type_t type, kz_syscall_param_t *param);

/**
 * @brief ユーザ・スレッド
 */
int test08_1_main(int argc, char *argv[]);

#endif
