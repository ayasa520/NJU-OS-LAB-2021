
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define GLOBAL_VARIABLES_HERE

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "proc.h"
#include "global.h"



PUBLIC	PROCESS			proc_table[NR_TASKS];

PUBLIC	char			task_stack[STACK_SIZE_TOTAL];

// PUBLIC	TASK	task_table[NR_TASKS] = {{TestA, STACK_SIZE_TESTA, "TestA"},
// 					{TestB, STACK_SIZE_TESTB, "TestB"},
// 					{TestC, STACK_SIZE_TESTC, "TestC"}};

#ifdef READERS_FIRST
PUBLIC TASK task_table[NR_TASKS]= {
    {r1r,STACK_SIZE_TESTA,"A"},
    {r1r,STACK_SIZE_TESTA,"B"},
    {r1r,STACK_SIZE_TESTA,"C"},
    {r1w,STACK_SIZE_TESTA,"D"},
    {r1w,STACK_SIZE_TESTA,"E"},
    {F,STACK_SIZE_TESTA,"F"},
    {G,STACK_SIZE_TESTA,"G"}
};
#endif
#ifdef WRITERS_FIRST
PUBLIC TASK task_table[NR_TASKS]= {
    {w1r,STACK_SIZE_TESTA,"A"},
    {w1r,STACK_SIZE_TESTA,"B"},
    {w1r,STACK_SIZE_TESTA,"C"},
    {w1w,STACK_SIZE_TESTA,"D"},
    {w1w,STACK_SIZE_TESTA,"E"},
    {F,STACK_SIZE_TESTA,"F"},
    {G,STACK_SIZE_TESTA,"G"}
};
#endif
#ifdef READERS_WRITERS
PUBLIC TASK task_table[NR_TASKS]= {
    {rwr,STACK_SIZE_TESTA,"A"},
    {rwr,STACK_SIZE_TESTA,"B"},
    {rwr,STACK_SIZE_TESTA,"C"},
    {rww,STACK_SIZE_TESTA,"D"},
    {rww,STACK_SIZE_TESTA,"E"},
    {F,STACK_SIZE_TESTA,"F"},
    {G,STACK_SIZE_TESTA,"G"}
};
#endif

PUBLIC	irq_handler		irq_table[NR_IRQ];

PUBLIC	system_call		sys_call_table[NR_SYS_CALL] = {sys_get_ticks,sys_print,sys_sleep,sys_wait,sys_signal,sys_clear};

PUBLIC SEMAPHORE fileSrcLock;
PUBLIC SEMAPHORE readWriteLock;
PUBLIC SEMAPHORE readCntChangeLock; // 保证互斥修改
PUBLIC SEMAPHORE readCntLock;   // 最多几个人读, 加载最外面
PUBLIC SEMAPHORE writeCntChangeLock;
PUBLIC SEMAPHORE mutex2Lock;    // 名字不好取... 保证最多只有一个读者与写者竞争 mutexLock
PUBLIC SEMAPHORE mutexLock;     // 有写者在写或者等待写的标志
PUBLIC int writeCnt=0;
PUBLIC int readCnt=0;
PUBLIC int readOrWrite=-1;   // 0 读, 1 写
PUBLIC QUEUE readyQueue;

PUBLIC int find(QUEUE*q,int pid){
    for (int i = 0; i < q->length; i++)
    {
        if(q->procs[(q->begin+i)%NR_TASKS]->pid==pid){
            return 1;
        }
    }
    return 0;
}
PUBLIC void enqueue(QUEUE *q,PROCESS*proc){
    q->procs[(q->begin+q->length++)%NR_TASKS] = proc;
}
PUBLIC PROCESS* dequeue(QUEUE*q){
    PROCESS* p;
    q->length--;
    p = q->procs[q->begin];
    q->begin = (q->begin+1)%NR_TASKS;
    return p;
}

PUBLIC void initQueue(QUEUE*q){
    q->begin=0;
	q->length=0;
}
PUBLIC PROCESS* getFront(QUEUE*q){
    return q->procs[q->begin];
}
PUBLIC PROCESS* getBack(QUEUE*q){
    return q->procs[(q->begin+q->length-1)*NR_TASKS];
}
PUBLIC void initSemaphore(SEMAPHORE*s,int val){
    s->value=val;
    initQueue(&(s->queue));
}
PUBLIC char prompts[][20] = {
    "%s %s begin.\n",
    "%s %s.\n",
    "%s %s end.\n",
    "read",
    "write",
    "%s write\n",
    "%s read %d\n"
};
