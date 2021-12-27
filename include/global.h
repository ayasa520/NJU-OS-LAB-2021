
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* EXTERN is defined as extern except in global.c */
#ifdef	GLOBAL_VARIABLES_HERE
#undef	EXTERN
#define	EXTERN
#endif

EXTERN	int		ticks;

EXTERN	int		disp_pos;
EXTERN	u8		gdt_ptr[6];	// 0~15:Limit  16~47:Base
EXTERN	DESCRIPTOR	gdt[GDT_SIZE];
EXTERN	u8		idt_ptr[6];	// 0~15:Limit  16~47:Base
EXTERN	GATE		idt[IDT_SIZE];

EXTERN	u32		k_reenter;

EXTERN	TSS		tss;
EXTERN	PROCESS*	p_proc_ready;

extern	PROCESS		proc_table[];
extern	char		task_stack[];
extern  TASK            task_table[];
extern	irq_handler	irq_table[];


extern SEMAPHORE fileSrcLock;
extern SEMAPHORE readCntChangeLock;
extern SEMAPHORE readCntLock;
extern SEMAPHORE writeCntChangeLock;
extern SEMAPHORE mutex2Lock;
extern SEMAPHORE mutexLock;
extern SEMAPHORE readWriteLock;

extern int writeCnt;
extern int readCnt;

extern QUEUE readyQueue;

extern int find(QUEUE*q,int pid);
extern void enqueue(QUEUE *q,PROCESS*proc);
extern PROCESS* dequeue(QUEUE*q);
extern void initQueue(QUEUE*q);
extern void initSemaphore(SEMAPHORE*s,int val);
extern PROCESS* getFront(QUEUE*q);
extern PROCESS* getBack(QUEUE*q);
extern int readOrWrite;
extern char prompts[][20]; 