#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#ifdef MLFQ
queue fbqs[QUECOUNT];
void initQueues()
{
    for (int i = 0; i < QUECOUNT; i++)
    {
        fbqs[i].top = -1;
        fbqs[i].capacity = NPROC;
        fbqs[i].procCount = 0;
    }
    fbqs[0].sliceTime = SLICETTIME0;
    fbqs[1].sliceTime = SLICETTIME1;
    fbqs[2].sliceTime = SLICETTIME2;
    fbqs[3].sliceTime = SLICETTIME3;
}
int isFull(int queueNumber)
{
    return (fbqs[queueNumber].procCount == fbqs[queueNumber].capacity);
}
int isEmpty(int queueNumber)
{
    return (fbqs[queueNumber].procCount == 0);
}
void enque(struct proc *p, int queueNumber)
{
    if (isFull(queueNumber))
    {
        return;
    }
    p->isQueuedFlag = 1;
    p->enquedAtTick = ticks;
    p->queueIndex = queueNumber;
    ++fbqs[queueNumber].top;
    fbqs[queueNumber].procList[fbqs[queueNumber].top] = p;
    ++fbqs[queueNumber].procCount;
}
void remove(struct proc *p)
{
    int queueNumber = p->queueIndex;
    p->isQueuedFlag = 0;
    if (isEmpty(queueNumber))
    {
        return;
    }
    --fbqs[queueNumber].procCount;
    --fbqs[queueNumber].top;
    for (int i = 0; i <= fbqs[queueNumber].top; i++)
    {
        if (fbqs[queueNumber].procList[i] == p)
        {
            for (int j = i; j < fbqs[queueNumber].top; j++)
            {
                fbqs[queueNumber].procList[j] = fbqs[queueNumber].procList[j + 1];
            }
            break;
        }
    }
}
struct proc *deque(int queueNumber)
{
    struct proc *p = fbqs[queueNumber].procList[0];
    remove(p);
    return p;
}
void shiftDown(struct proc *p)
{
    int newQueueNumber = p->queueIndex + 1;
    p->tickedFor = 0;
    remove(p);
    enque(p, newQueueNumber);
}
void shiftUp(struct proc *p)
{
    int newQueueNumber = p->queueIndex - 1;
    p->tickedFor = 0;
    remove(p);
    enque(p, newQueueNumber);
}
void insetAtBack(struct proc *p)
{
    int newQueueNumber = p->queueIndex;
    int enqueTemp = p->enquedAtTick;
    int tickTemp = p->tickedFor;
    remove(p);
    if (p->state == RUNNABLE)
    {
        enque(p, newQueueNumber);
        p->tickedFor = tickTemp;
        p->enquedAtTick = enqueTemp;
    }
}
#endif