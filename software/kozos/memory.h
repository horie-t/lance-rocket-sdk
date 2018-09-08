#ifndef _MEMORY_H_INCLUDED_
#define _MEMORY_H_INCLUDED_

/**
 * @brief 動的メモリの初期化
 */
int kzmem_init(void);

/**
 * @brief 動的メモリの獲得
 */
void *kzmem_alloc(int size);

/**
 * @brief 動的メモリの解放
 */
void kzmem_free(void *mem);

#endif
