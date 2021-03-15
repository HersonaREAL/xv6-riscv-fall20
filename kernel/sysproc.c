#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64 
sys_sigalarm(void){
  int boomTick;
  uint64 boomb;
  struct proc *p = myproc();
  if (argint(0, &boomTick) < 0)
    return -1;
  if(argaddr(1,&boomb)<0)
    return -1;
  if(boomTick==0&&boomb==0){
    p->boomReady = 0;
    p->boomTime = 0;
    p->dida = 0;
    return 0;
  }
  p->boomTime = boomTick;
  p->boomb = boomb;
  p->dida = 0;
  p->boomReady = 1;
  return 0;
}
uint64 sys_sigreturn(void){
  struct proc *p = myproc();
  p->trapframe->kernel_satp = p->bak.kernel_satp;
  p->trapframe->kernel_sp    =  p->bak.kernel_sp;      
  p->trapframe->kernel_trap  =  p->bak.kernel_trap;  
  p->trapframe->epc          =  p->bak.epc;
  p->trapframe->kernel_hartid = p->bak.kernel_hartid;
  p->trapframe->ra =  p->bak.ra;
  p->trapframe->sp =  p->bak.sp;
  p->trapframe->gp =  p->bak.gp;
  p->trapframe->tp =  p->bak.tp;
  p->trapframe->t0 =  p->bak.t0;
  p->trapframe->t1 =  p->bak.t1;
  p->trapframe->t2 =  p->bak.t2;
  p->trapframe->s0 =  p->bak.s0;
  p->trapframe->s1 =  p->bak.s1;
  p->trapframe->a0 =  p->bak.a0;
  p->trapframe->a1 =  p->bak.a1;
  p->trapframe->a2 =  p->bak.a2;
  p->trapframe->a3 =  p->bak.a3;
  p->trapframe->a4 =  p->bak.a4;
  p->trapframe->a5 =  p->bak.a5;
  p->trapframe->a6 =  p->bak.a6;
  p->trapframe->a7 =  p->bak.a7;
  p->trapframe->s2 =  p->bak.s2;
  p->trapframe->s3 =  p->bak.s3;
  p->trapframe->s4 =  p->bak.s4;
  p->trapframe->s5 =  p->bak.s5;
  p->trapframe->s6 =  p->bak.s6;
  p->trapframe->s7 =  p->bak.s7;
  p->trapframe->s8 =  p->bak.s8;
  p->trapframe->s9 =  p->bak.s9;
  p->trapframe->s10 =  p->bak.s10;
  p->trapframe->s11 =  p->bak.s11;
  p->trapframe->t3 =  p->bak.t3;
  p->trapframe->t4 =  p->bak.t4;
  p->trapframe->t5 =  p->bak.t5;
  p->trapframe->t6 =  p->bak.t6;
  p->boomReady = 1;
  return 0;
}

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  backtrace();
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
