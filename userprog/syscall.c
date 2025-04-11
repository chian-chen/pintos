#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

#include <devices/shutdown.h>

#include <string.h>
#include <filesys/file.h>
#include <devices/input.h>
#include <threads/malloc.h>
#include <threads/palloc.h>
#include "process.h"
#include "pagedir.h"
#include <threads/vaddr.h>
#include <filesys/filesys.h>

#define MAX_SYSCALL 20

// lab01 Hint - Here are the system calls you need to implement.

/* System call for process. */

void sys_halt(void);
void sys_exit(struct intr_frame* f);
void sys_exec(struct intr_frame* f);
void sys_wait(struct intr_frame* f);

/* System call for file. */
void sys_create(struct intr_frame* f);
void sys_remove(struct intr_frame* f);
void sys_open(struct intr_frame* f);
void sys_filesize(struct intr_frame* f);
void sys_read(struct intr_frame* f);
void sys_write(struct intr_frame* f);
void sys_seek(struct intr_frame* f);
void sys_tell(struct intr_frame* f);
void sys_close(struct intr_frame* f);


static void (*syscalls[MAX_SYSCALL])(struct intr_frame *) = {
  [SYS_HALT] = sys_halt,
  [SYS_EXIT] = sys_exit,
  [SYS_EXEC] = sys_exec,
  [SYS_WAIT] = sys_wait,
  [SYS_CREATE] = sys_create,
  [SYS_REMOVE] = sys_remove,
  [SYS_OPEN] = sys_open,
  [SYS_FILESIZE] = sys_filesize,
  [SYS_READ] = sys_read,
  [SYS_WRITE] = sys_write,
  [SYS_SEEK] = sys_seek,
  [SYS_TELL] = sys_tell,
  [SYS_CLOSE] = sys_close
};

static void syscall_handler (struct intr_frame *);

void syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}


static int 
get_user (const uint8_t *uaddr)
{
  int result;
  asm ("movl $1f, %0; movzbl %1, %0; 1:" : "=&a" (result) : "m" (*uaddr));
  return result;
}

/* general check if the ptr is valid or not */
void *
check_ptr_valid(const void *vaddr, uint8_t offset)
{
  if (!is_user_vaddr(vaddr))
  {
    error_exit();
  }
  void *ptr = pagedir_get_page (thread_current()->pagedir, vaddr);
  if(!is_user_vaddr(vaddr) || pagedir_get_page (thread_current()->pagedir, vaddr) == NULL)
  {
    error_exit();
  }
  uint8_t *check_byteptr = (uint8_t *) vaddr;
  for (uint8_t i = 0; i < offset; i++) 
  {
    if (get_user(check_byteptr + i) == -1)
    {
      error_exit ();
    }
  }
  return ptr;
}

/* System Call: void halt (void)
    Terminates Pintos by calling shutdown_power_off() (declared in devices/shutdown.h). 
*/
void sys_halt(void)
{
  shutdown_power_off();
}

/* 系統呼叫: exit - 結束程序，並傳回 exit status */
void sys_exit(struct intr_frame *f)
{
  uint32_t *user_ptr = f->esp;
  check_ptr_valid(user_ptr + 1, 4);
  *user_ptr++;
  // printf("Here the exit_status is: %d\n", *user_ptr);
  /* record the exit status of the process */
  thread_current()->exit_status = *user_ptr;
  thread_exit ();
}

/* 系統呼叫: exec - 執行命令 (未實作) */
void sys_exec(struct intr_frame *f)
{
  /* Not implemented. */
  uint32_t *user_ptr = f->esp;
  check_ptr_valid(user_ptr + 1, 4);
  check_ptr_valid(*(user_ptr + 1), 4);
  *user_ptr++;
  f->eax = process_execute((char*)* user_ptr);
}

/* 系統呼叫: wait - 等待程序結束 (未實作) */
void sys_wait(struct intr_frame *f)
{
  /* Not implemented. */
  uint32_t *user_ptr = f->esp;
  check_ptr_valid(user_ptr + 1, 4);
  *user_ptr++;
  f->eax = process_wait(*user_ptr);
}

/* 系統呼叫: create - 建立檔案 (未實作) */
void sys_create(struct intr_frame *f)
{
  /* Not implemented. */
  uint32_t *user_ptr = f->esp;
  thread_current()->exit_status = *user_ptr;
  thread_exit();
}

/* 系統呼叫: remove - 刪除檔案 (未實作) */
void sys_remove(struct intr_frame *f)
{
  /* Not implemented. */
  uint32_t *user_ptr = f->esp;
  thread_current()->exit_status = *user_ptr;
  thread_exit();
}

/* 系統呼叫: open - 開啟檔案 (未實作) */
void sys_open(struct intr_frame *f)
{
  /* Not implemented. */
  uint32_t *user_ptr = f->esp;
  thread_current()->exit_status = *user_ptr;
  thread_exit();
}

/* 系統呼叫: filesize - 取得檔案大小 (未實作) */
void sys_filesize(struct intr_frame *f)
{
  /* Not implemented. */
  uint32_t *user_ptr = f->esp;
  thread_current()->exit_status = *user_ptr;
  thread_exit();
}

/* 系統呼叫: read - 讀取檔案 (未實作) */
void sys_read(struct intr_frame *f)
{
  /* Not implemented. */
  uint32_t *user_ptr = f->esp;
  thread_current()->exit_status = *user_ptr;
  thread_exit();
}

/* 系統呼叫: write - 寫入資料
   若 fd == 1 (stdout) 則用 putbuf 印出，否則視為檔案寫入（未實作） */
void sys_write(struct intr_frame *f)
{
  uint32_t *user_ptr = f->esp;
  check_ptr_valid(user_ptr + 7, 4);
  check_ptr_valid(*(user_ptr + 6), 4);

  *user_ptr++;
  int fd = *user_ptr;
  const char *buffer = (const char *)*(user_ptr + 1);
  unsigned size = *(user_ptr + 2);
  // printf("buffer: %s\n", buffer);

  if (fd == 1) {
    putbuf(buffer, size);
    f->eax = size;
  } else {
    /* Not implemented for file writing */
    f->eax = -1;
  }
}

/* 系統呼叫: seek - 變更檔案讀寫位置 (未實作) */
void sys_seek(struct intr_frame *f)
{
  /* Not implemented. */
  uint32_t *user_ptr = f->esp;
  thread_current()->exit_status = *user_ptr;
  thread_exit();
}

/* 系統呼叫: tell - 取得當前檔案讀寫位置 (未實作) */
void sys_tell(struct intr_frame *f)
{
  /* Not implemented. */
  uint32_t *user_ptr = f->esp;
  thread_current()->exit_status = *user_ptr;
  thread_exit();
}

/* 系統呼叫: close - 關閉檔案 (未實作) */
void sys_close(struct intr_frame *f)
{
  /* Not implemented. */
  uint32_t *user_ptr = f->esp;
  thread_current()->exit_status = *user_ptr;
  thread_exit();
}

void error_exit(void)
{
  thread_current()->exit_status = -1;
  thread_exit();
}

static void syscall_handler (struct intr_frame *f UNUSED) 
{
  // printf ("system call!\n");
  uint32_t *user_ptr = f->esp;
  check_ptr_valid (user_ptr + 1, 4);
  
  int type = * (int *)f->esp;
  // printf("syscall number: %d\n", type);

  if(type <= 0 || type >= MAX_SYSCALL){
    // printf("Invalid syscall number: %d\n", type);
    error_exit();
  }
  syscalls[type](f);
}
