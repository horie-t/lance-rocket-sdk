#include "defines.h"
#include "encoding.h"
#include "kozos.h"
#include "interrupt.h"
#include "syscall.h"
#include "lib.h"

#define THREAD_NUM 6		/* TCBの個数 */
#define THREAD_NAME_SIZE 15	/* スレッド名の最大長 */

/**
 * @brief スレッド・コンテキスト
 */
typedef struct _kz_context {
  uint32_t sp;			/* スタック・ポインタ */
} kz_context;

/**
 * @brief タスク・コントロール・ブロック(TCB)
 */
typedef struct _kz_thread {
  struct _kz_thread *next;	   /* レディー・キューの次エントリ */
  char name[THREAD_NAME_SIZE + 1]; /* スレッド名 */
  char *stack;

  /* スレッドのスタート・アップ(thread_init()に渡すパラメータ)。メイン関数のパラメータ */
  struct {
    kz_func_t func;		/* スレッドのメイン関数 */
    int argc;			/* メイン関数のargc */
    char **argv;		/* メイン関数のargv */
  } init;

  /* システム・コール用バッファ */
  struct {
    kz_syscall_type_t type;
    kz_syscall_param_t *param;
  } syscall;

  /* スレッドのコンテキスト情報 */
  kz_context context;
} kz_thread;

/* スレッドのレディー・キュー */
static struct {
  kz_thread *head;
  kz_thread *tail;
} readyque;

/* 現在のスレッド */
static kz_thread *current;

/* タスク・コントロール・ブロック */
static kz_thread threads[THREAD_NUM];

void dispatch(kz_context *context);

/**
 * 現在のスレッドをレディー・キューから取り除く。removeの方が適切?
 */
static int getcurrent(void)
{
  if (current == NULL) {
    return -1;
  }

  readyque.head = current->next;
  if (readyque.head == NULL) {
    readyque.tail = NULL;
  }
  current->next = NULL;

  return 0;
}

/**
 * 現在のスレッドをレディー・キューの末尾に追加する
 */
static int putcurrent(void)
{
  if (current == NULL) {
    return -1;
  }

  if (readyque.tail) {
    readyque.tail->next = current;
  } else {
    readyque.head = current;
  }
  readyque.tail = current;

  return 0;
}

/**
 * @brief スレッドの終了
 */
static void thread_end(void)
{
  kz_exit();
}

/**
 * @brief スレッドのスタート・アップ
 */
static void thread_init(kz_thread *thp)
{
  thp->init.func(thp->init.argc, thp->init.argv);
  thread_end();
}

/*****************************************
 * システム・コールの処理
 *****************************************/
/**
 * @brief スレッドの起動(kz_run())
 */
static kz_thread_id_t thread_run(kz_func_t func, char *name, int stacksize, int argc, char *argv[])
{
  int i;
  kz_thread *thp;
  uint32_t *sp;
  extern char userstack;	/* リンカ・スクリプトで定義 */
  static char *thread_stack = &userstack;

  /* 空いているTCBを検索 */
  for (i = 0; i < THREAD_NUM; i ++) {
    thp = &threads[i];
    if (!thp->init.func)
      /* 見つかった */
      break;
  }
  if (i == THREAD_NUM)
    /* 見つからなかった */
    return -1;

  /* TCBの初期化 */
  memset(thp, 0, sizeof(*thp));
  thp->next = NULL;

  thp->init.func = func;
  thp->init.argc = argc;
  thp->init.argv = argv;

  /* スタック領域を確保し、TCBに設定 */
  memset(thread_stack, 0, stacksize);
  thread_stack += stacksize;
  thp->stack = thread_stack;

  sp = (uint32_t *)thp->stack;
  *(--sp) = (uint32_t)thread_end;

  /* プログラム・カウンタを設定する */
  *(--sp) = (uint32_t)thread_init;

  *(--sp) = 0;			/* x31 */
  *(--sp) = 0;			/* x30 */
  *(--sp) = 0;			/* x29 */
  *(--sp) = 0;			/* x28 */
  *(--sp) = 0;			/* x17 */
  *(--sp) = 0;			/* x16 */
  *(--sp) = 0;			/* x15 */
  *(--sp) = 0;			/* x14 */
  *(--sp) = 0;			/* x13 */
  *(--sp) = 0;			/* x12 */
  *(--sp) = 0;			/* x11 */
  *(--sp) = (uint32_t)thp;	/* x10(a1)スレッドのスタート・アップに渡す引数 */
  *(--sp) = 0;			/* x7 */
  *(--sp) = 0;			/* x6 */
  *(--sp) = 0;			/* x5 */
  *(--sp) = 0;			/* x1 */

  /* スレッドのコンテキストを設定 */
  thp->context.sp = (uint32_t)sp;

  /* システム・コールを呼び出したスレッドをレディー・キューに入れる */
  putcurrent();

  /* 新規作成したスレッドをレディー・キューに入れる */
  current = thp;
  putcurrent();

  return (kz_thread_id_t) current;
}

/**
 * @brief スレッドの終了(kz_exit())
 */
static int thread_exit(void)
{
  /* 「<スレッド名> EXIT.」を出力して終了*/
  puts(current->name);
  puts(" EXIT.\n");
  
  memset(current, 0, sizeof(*current));
  
  return 0;
}

static void call_functions(kz_syscall_type_t type, kz_syscall_param_t *p)
{
  /* システム・コール実行中にcurrentが書き換わるので注意 */
  switch (type) {
  case KZ_SYSCALL_TYPE_RUN:
    p->un.run.ret = thread_run(p->un.run.func, p->un.run.name,
			       p->un.run.stacksize,
			       p->un.run.argc, p->un.run.argv);
    break;
  case KZ_SYSCALL_TYPE_EXIT:
    thread_exit();
    break;
  default:
    break;
  }
}

/**
 * @brief システム・コールの処理 
 */
static void syscall_proc(kz_syscall_type_t type, kz_syscall_param_t *p)
{
  getcurrent();
  call_functions(type, p);
}

/**
 * @brief スレッドのスケジューリング
 */
static void schedule(void)
{
  if (!readyque.head)
    kz_sysdown();

  current = readyque.head;
}

/**
 * @brief システム・コール呼び出し
 */
static void syscall_intr(void)
{
  syscall_proc(current->syscall.type, current->syscall.param);
}

/**
 * @brief ソフトウェア・エラーの発生
 */
static void softerr_intr(void)
{
  puts(current->name);
  puts(" DOWN.\n");

  getcurrent();
  thread_exit();
}

/**
 * @brief 例外ハンドラ
 */
void handle_sync_trap(uint32_t sp)
{
  current->context.sp = sp;
  
  int32_t mcause = read_csr(mcause);
  switch (mcause) {
  case CAUSE_MACHINE_ECALL:
    syscall_intr();
    break;
  default:
    softerr_intr();
  }
  
  schedule();
  dispatch(&current->context);
}

void kz_start(kz_func_t func, char *name, int stacksize, int argc, char *argv[])
{
  current = NULL;

  readyque.head = readyque.tail = NULL;
  memset(threads, 0, sizeof(threads));
  
  mtvec_init();

  current = (kz_thread *)thread_run(func, name, stacksize, argc, argv);
  dispatch(&current->context);
  // ここには返って来ない。
}

void kz_sysdown(void)
{
  puts("system error!\n");
  while (1)
    ;
}

void kz_syscall(kz_syscall_type_t type, kz_syscall_param_t *param)
{
  current->syscall.type = type;
  current->syscall.param = param;

  asm volatile ("ecall");
}

  
