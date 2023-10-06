#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
// #ifdef MLFQ
void insert(struct proc *p,int index)
{
    acquire(&p->lock);
    release(&p->lock);

}
// #endif