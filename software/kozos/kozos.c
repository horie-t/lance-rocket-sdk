#include "defines.h"
#include "encoding.h"
#include "kozos.h"
#include "intr.h"
#include "interrupt.h"
#include "syscall.h"
#include "memory.h"
#include "lib.h"

#define THREAD_NUM 6		/* TCBの個数 */
#define PRIORITY_NUM 16		/* 優先度の数 */
#define THREAD_NAME_SIZE 15	/* スレッド名の最大長 */

/**
 * @brief スレッド・コンテキスト
 */
typedef struct _kz_context {
  uint32_t *sp;			/* スタック・ポインタ */
} kz_context;

/**
 * @brief タスク・コントロール・ブロック(TCB)
 */
typedef struct _kz_thread {
  struct _kz_thread *next;	   /* レディー・キューの次エントリ */
  char name[THREAD_NAME_SIZE + 1]; /* スレッド名 */
  int priority;			   /* 優先度 */
  char *stack;
  uint32_t flags;		/* 各種フラグ (0:READYフラグ)*/
#define KZ_THREAD_FLAG_READY 	(1 << 0)

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

/**
 * @brief メッセージ・バッファ
 */
typedef struct _kz_msgbuf {
  struct _kz_msgbuf *next;
  kz_thread *sender;		/* メッセージを送信したスレッド */
  struct {
    int size;
    char *p;
  } param;
} kz_msgbuf;

/**
 * @brief メッセージ・ボックス
 */
typedef struct _kz_msgbox {
  kz_thread *receiver;
  kz_msgbuf *head;
  kz_msgbuf *tail;
} kz_msgbox;

/* スレッドのレディー・キュー */
static struct {
  kz_thread *head;
  kz_thread *tail;
} readyque[PRIORITY_NUM];

/* 現在のスレッド */
static kz_thread *current;

/* タスク・コントロール・ブロック */
static kz_thread threads[THREAD_NUM];

/* 割込みハンドラ */
static kz_handler_t handlers[SOFTVEC_TYPE_NUM];

/* メッセージ・ボックス */
static kz_msgbox msgboxes[MSGBOX_ID_NUM];

void dispatch(kz_context *context);

/**
 * 現在のスレッドをレディー・キューから取り除く。removeの方が適切?
 */
static int getcurrent(void)
{
  if (current == NULL) {
    return -1;
  }
  if (!(current->flags & KZ_THREAD_FLAG_READY)) {
    return -1;
  }

  readyque[current->priority].head = current->next;
  if (readyque[current->priority].head == NULL) {
    readyque[current->priority].tail = NULL;
  }
  current->flags &= ~KZ_THREAD_FLAG_READY;
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
  if (current->flags & KZ_THREAD_FLAG_READY) {
    /* 既に追加済みのはず */
    return 1;
  }

  if (readyque[current->priority].tail) {
    readyque[current->priority].tail->next = current;
  } else {
    readyque[current->priority].head = current;
  }
  readyque[current->priority].tail = current;
  current->flags |= KZ_THREAD_FLAG_READY;

  return 0;
}
static void sendmsg(kz_msgbox *mboxp, kz_thread *thp, int size, char *p)
{
  kz_msgbuf *mp;

  /* メッセージ・バッファの作成 */
  mp = (kz_msgbuf *)kzmem_alloc(sizeof(*mp));
  if (mp == NULL)
    kz_sysdown();
  mp->next       = NULL;
  mp->sender     = thp;
  mp->param.size = size;
  mp->param.p    = p;

  /* メッセージ・ボックスの末尾にメッセージを接続する */
  if (mboxp->tail) {
    mboxp->tail->next = mp;
  } else {
    mboxp->head = mp;
  }
  mboxp->tail = mp;
}

static void recvmsg(kz_msgbox *mboxp)
{
  kz_msgbuf *mp;
  kz_syscall_param_t *p;

  /* メッセージ・ボックスの先頭にあるメッセージを抜き出す */
  mp = mboxp->head;
  mboxp->head = mp->next;
  if (mboxp->head == NULL)
    mboxp->tail = NULL;
  mp->next = NULL;

  /* メッセージを受信するスレッドに返す値を設定する */
  p = mboxp->receiver->syscall.param;
  p->un.recv.ret = (kz_thread_id_t)mp->sender;
  if (p->un.recv.sizep)
    *(p->un.recv.sizep) = mp->param.size;
  if (p->un.recv.pp)
    *(p->un.recv.pp) = mp->param.p;

  /* 受信待ちスレッドはいなくなったのでNULLに戻す */
  mboxp->receiver = NULL;

  /* メッセージ・バッファの解放 */
  kzmem_free(mp);
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
static kz_thread_id_t thread_run(kz_func_t func, char *name, int priority, int stacksize, int argc, char *argv[])
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
  strcpy(thp->name, name);
  thp->next = NULL;
  thp->priority = priority;
  thp->flags = 0;

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
  *(--sp) = priority ? 0 : MSTATUS_MIE;

  *(--sp) = 0;			/* x31 */
  *(--sp) = 0;			/* x30 */
  *(--sp) = 0;			/* x29 */
  *(--sp) = 0;			/* x28 */
  *(--sp) = 0;			/* x27 */
  *(--sp) = 0;			/* x26 */
  *(--sp) = 0;			/* x25 */
  *(--sp) = 0;			/* x24 */
  *(--sp) = 0;			/* x23 */
  *(--sp) = 0;			/* x22 */
  *(--sp) = 0;			/* x21 */
  *(--sp) = 0;			/* x20 */
  *(--sp) = 0;			/* x19 */
  *(--sp) = 0;			/* x18 */
  *(--sp) = 0;			/* x17 */
  *(--sp) = 0;			/* x16 */
  *(--sp) = 0;			/* x15 */
  *(--sp) = 0;			/* x14 */
  *(--sp) = 0;			/* x13 */
  *(--sp) = 0;			/* x12 */
  *(--sp) = 0;			/* x11 */
  *(--sp) = (uint32_t)thp;	/* x10(a0)スレッドのスタート・アップに渡す引数 */
  *(--sp) = 0;			/* x9 */
  *(--sp) = 0;			/* x8 */
  *(--sp) = 0;			/* x7 */
  *(--sp) = 0;			/* x6 */
  *(--sp) = 0;			/* x5 */
  *(--sp) = 0;			/* x4 */
  *(--sp) = 0;			/* x3 */
  *(--sp) = 0;			/* x2 */
  *(--sp) = 0;			/* x1 */
  *(--sp) = 0;			/* x0 */

  /* スレッドのコンテキストを設定 */
  thp->context.sp = sp;

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

/**
 * @brief スレッドの実行権を他に渡す(kz_wait())
 */
static int thread_wait(void)
{
  putcurrent();
  return 0;
}

/**
 * @brief スレッドの待機状態への遷移(kz_sleep())
 */
static int thread_sleep(void)
{
  /* readyqueからなくなった状態続くようになる */
  return 0;
}

/**
 * @brief 指定したスレッドを実行可能状態に遷移させる(kz_wakeup())
 */
static int thread_wakeup(kz_thread_id_t id)
{
  /* wakeupを呼び出したスレッドをreadyqueに戻す */
  putcurrent();

  /* 指定されたスレッドをreadyqueに戻して、実行されるようにする */
  current = (kz_thread *)id;
  putcurrent();

  return 0;
}

/**
 * @brief 自スレッドのIDを取得する(kz_getid())
 */
static kz_thread_id_t thread_getid(void)
{
  putcurrent();
  return (kz_thread_id_t)current;
}

static int thread_chpri(int priority)
{
  int old = current->priority;
  if (priority >= 0)
    current->priority = priority;
  putcurrent();
  return old;
}

static void *thread_kmalloc(int size)
{
  putcurrent();
  return kzmem_alloc(size);
}

static int thread_kmfree(char *p)
{
  kzmem_free(p);
  putcurrent();
  return 0;
}

static int thread_send(kz_msgbox_id_t id, int size, char *p)
{
  kz_msgbox *mboxp = &msgboxes[id];

  putcurrent();
  sendmsg(mboxp, current, size, p);

  /* 受信待ちスレッドが存在している場合には受信処理を行う。 */
  if (mboxp->receiver) {
    current = mboxp->receiver;
    recvmsg(mboxp);
    putcurrent();
  }

  return size;
}

static kz_thread_id_t thread_recv(kz_msgbox_id_t id, int *sizep, char **pp)
{
  kz_msgbox *mboxp = &msgboxes[id];

  if (mboxp->receiver)
    /* 他スレッドがすでに受信待ちしている */
    kz_sysdown();

  mboxp->receiver = current;

  if (mboxp->head == NULL) {
    /* メッセージ・ボックスにメッセージがないので、スレッドをスリープさせる */
    return -1;
  }

  /* メッセージを受信し、レディー状態に戻す */
  recvmsg(mboxp);
  putcurrent();

  return current->syscall.param->un.recv.ret;
}

static int thread_setintr(softvec_type_t type, kz_handler_t handler)
{
  handlers[type] = handler;
  putcurrent();

  return 0;
}


static void call_functions(kz_syscall_type_t type, kz_syscall_param_t *p)
{
  /* システム・コール実行中にcurrentが書き換わるので注意 */
  switch (type) {
  case KZ_SYSCALL_TYPE_RUN:
    p->un.run.ret = thread_run(p->un.run.func, p->un.run.name,
			       p->un.run.priority, p->un.run.stacksize,
			       p->un.run.argc, p->un.run.argv);
    break;
  case KZ_SYSCALL_TYPE_EXIT:
    thread_exit();
    break;
  case KZ_SYSCALL_TYPE_WAIT:
    p->un.wait.ret = thread_wait();
    break;
  case KZ_SYSCALL_TYPE_SLEEP:
    p->un.sleep.ret = thread_sleep();
    break;
  case KZ_SYSCALL_TYPE_WAKEUP:
    p->un.wakeup.ret = thread_wakeup(p->un.wakeup.id);
    break;
  case KZ_SYSCALL_TYPE_GETID:
    p->un.getid.id = thread_getid();
    break;
  case KZ_SYSCALL_TYPE_CHPRI:
    p->un.chpri.ret = thread_chpri(p->un.chpri.priority);
    break;
  case KZ_SYSCALL_TYPE_KMALLOC:
    p->un.kmalloc.ret = thread_kmalloc(p->un.kmalloc.size);
    break;
  case KZ_SYSCALL_TYPE_KMFREE:
    p->un.kmfree.ret = thread_kmfree(p->un.kmfree.p);
    break;
  case KZ_SYSCALL_TYPE_SEND:
    p->un.send.ret = thread_send(p->un.send.id, p->un.send.size, p->un.send.p);
    break;
  case KZ_SYSCALL_TYPE_RECV:
    p->un.recv.ret = thread_recv(p->un.recv.id, p->un.recv.sizep, p->un.recv.pp);
    break;
  case KZ_SYSCALL_TYPE_SETINTR:
    p->un.setintr.ret = thread_setintr(p->un.setintr.type, p->un.setintr.handler);
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
 * @brief サービス・コールの処理
 */
static void srvcall_proc(kz_syscall_type_t type, kz_syscall_param_t *p)
{
  current = NULL;
  call_functions(type, p);
}

/**
 * @brief スレッドのスケジューリング
 */
static void schedule(void)
{
  int i;

  /*
   * 優先度の高い順(優先度の数値の低い順)にreadyqueを見て
   * 動作可能なスレッドを検索する
   */
  for (i = 0; i < PRIORITY_NUM; i++) {
    if (readyque[i].head)
      /* 見つかった */
      break;
  }
  if (i == PRIORITY_NUM)
    /* 見つからなかった */
    kz_sysdown();

  current = readyque[i].head;
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
static void softerr_intr(int32_t mcause, int32_t mepc)
{
  puts(current->name);
  puts(" DOWN. mcause: "); putxval(mcause, 0);
  puts(" mepc: "); putxval(mepc, 0); puts("\n");

  getcurrent();
  thread_exit();
}

/**
 * @brief 例外ハンドラ
 */
void handle_sync_trap(uint32_t *sp)
{
  current->context.sp = sp;
  
  int32_t mcause = read_csr(mcause);
  int32_t mepc = read_csr(mepc);
  
  switch (mcause) {
  case CAUSE_MACHINE_ECALL:
    *(sp+33) += 4;		/* ecall命令の次の命令から再開させる */
    syscall_intr();
    break;
  default:
    softerr_intr(mcause, mepc);
  }
  
  schedule();
  dispatch(&current->context);
}

/**
 * @brief 外部割込み
 */
void handle_m_external_interrupt(uint32_t *sp)
{
  // ここの値を一旦読んで完了を通知しないと、割込みが発生し続ける。
  uint32_t plic_intr_num = *(volatile uint32_t*)(PLIC_CTRL_ADDR + PLIC_CLAIM_OFFSET);

  current->context.sp = sp;
  
  handlers[SOFTVEC_TYPE_SERINTR]();
  
  schedule();
  *(volatile uint32_t*)(PLIC_CTRL_ADDR + PLIC_CLAIM_OFFSET) = plic_intr_num;
  dispatch(&current->context);
}

void kz_start(kz_func_t func, char *name, int priority, int stacksize, int argc, char *argv[])
{
  kzmem_init();
  
  current = NULL;

  memset(readyque, 0, sizeof(readyque));
  memset(threads, 0, sizeof(threads));
  memset(msgboxes, 0, sizeof(msgboxes));
  
  mtvec_init();

  current = (kz_thread *)thread_run(func, name, priority, stacksize, argc, argv);
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

void kz_srvcall(kz_syscall_type_t type, kz_syscall_param_t *param)
{
  srvcall_proc(type, param);
}
