
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"


/*======================================================================*
                            kernel_main
 *======================================================================*/

PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	TASK*		p_task		= task_table;
	PROCESS*	p_proc		= proc_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16		selector_ldt	= SELECTOR_LDT_FIRST;
	int i;
	for (i = 0; i < NR_TASKS; i++) {
		strcpy(p_proc->p_name, p_task->name);	// name of the process
		p_proc->pid = i;			// pid

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
		p_proc->regs.cs	= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ds	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.es	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.fs	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ss	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK)
			| RPL_TASK;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

	proc_table[0].ticks = proc_table[0].priority = 2*ROUND;
	proc_table[1].ticks = proc_table[1].priority =  3*ROUND;
	proc_table[2].ticks = proc_table[2].priority =  3*ROUND;
	proc_table[3].ticks = proc_table[3].priority =  3*ROUND;
	proc_table[4].ticks = proc_table[4].priority =  4*ROUND;
	proc_table[5].ticks = proc_table[5].priority =  4*ROUND;
	proc_table[6].ticks = proc_table[6].priority =  4*ROUND;


	k_reenter = 0;
	ticks = 0;

	p_proc_ready	= proc_table;

	initQueue(&readyQueue);
	initSemaphore(&readCntChangeLock,1);  // 互斥修改计数器
	initSemaphore(&readCntLock,3);  // 几个人同时读
	initSemaphore(&fileSrcLock,1);	// 互斥用文件
	initSemaphore(&writeCntChangeLock,1);	
	initSemaphore(&mutex2Lock,1);
	initSemaphore(&mutexLock,1);
	initSemaphore(&readWriteLock,1);



	for (int i = 0; i < NR_TASKS; i++)
	{
		proc_table[i].state=RUNNABLE;
		proc_table[i].ticks_used=0;
		enqueue(&readyQueue,&proc_table[i]);
	}
	
	init_clock();
    init_keyboard();

	restart();
                disp_pos = 0;
                for (int i = 0; i < 80 * 25; ++i) {
                        disp_str(" ");
                }
                disp_pos = 0;
	
	while(1){}
}

/**
 * @brief 读优先读
 * 
 */
void r1r(){
	while(1){
		_wait(&readCntLock);		// 保证同时读的人不超过要求
			_wait(&readCntChangeLock);	// 保证互斥修改人数
				if(!(readCnt++))
					_wait(&fileSrcLock);    // 允许同时多人读, 只有第一个读者需要申请资源
			_signal(&readCntChangeLock); 


			readOrWrite = 0;

		
			printf_color(prompts[0],p_proc_ready->pid+9,p_proc_ready->p_name,prompts[3]);
			// // 读
			printf_color(prompts[1],p_proc_ready->pid+9,p_proc_ready->p_name,prompts[3]);
			milli_delay(p_proc_ready->ticks*1000/HZ);
			printf_color(prompts[2],p_proc_ready->pid+9,p_proc_ready->p_name,prompts[3]);
			_wait(&readCntChangeLock);
				if(!(--readCnt))
					_signal(&fileSrcLock);
			_signal(&readCntChangeLock);
		_signal(&readCntLock);
		milli_delay(100);
	}
}

/**
 * @brief 读优先写
 * 
 */
void r1w(){
	while(1){
		_wait(&fileSrcLock); 	//直接申请资源就行了, 因为写写互斥, 读写互斥
			readOrWrite = 1;

   		printf_color(prompts[0],p_proc_ready->pid+9,p_proc_ready->p_name,prompts[4]);
			// // 读
		printf_color(prompts[1],p_proc_ready->pid+9,p_proc_ready->p_name,prompts[4]);
		milli_delay(p_proc_ready->ticks*1000/HZ);
		printf_color(prompts[2],p_proc_ready->pid+9,p_proc_ready->p_name,prompts[4]);
		_signal(&fileSrcLock);
		milli_delay(100);

	}
}

/**
 * @brief 写优先读
 * 
 */
void w1r(){
	while(1){
		_wait(&readCntLock);
			_wait(&mutex2Lock);		// 在 mutexLock 不能排长队, 最多只有一个读者与写者竞争, 保证写者占优势. 其余读者在 mutex2Lock 排队
				_wait(&mutexLock);	// 保证有写者在写或者在等待就不能读
					_wait(&readCntChangeLock);
						if(!(readCnt++))
							_wait(&fileSrcLock);
					_signal(&readCntChangeLock);
				_signal(&mutexLock); // 不释放的话不能多个读者同时读
			_signal(&mutex2Lock);
			readOrWrite = 0;

			// 读

			printf_color(prompts[0],p_proc_ready->pid+9,p_proc_ready->p_name,prompts[3]);
			// // 读
			printf_color(prompts[1],p_proc_ready->pid+9,p_proc_ready->p_name,prompts[3]);
			milli_delay(p_proc_ready->ticks*1000/HZ);
			printf_color(prompts[2],p_proc_ready->pid+9,p_proc_ready->p_name,prompts[3]);
			_wait(&readCntChangeLock);
				if(!(--readCnt))
					_signal(&fileSrcLock);
			_signal(&readCntChangeLock);
		_signal(&readCntLock);
		milli_delay(100);
	}
}

/**
 * @brief 写优先写
 * 
 */
void w1w(){
	while(1){
		_wait(&writeCntChangeLock);		// 互斥修改写者人数
			if(!(writeCnt++))
				_wait(&mutexLock);		// 保证只要有写者, 读者就无法读
		_signal(&writeCntChangeLock);
			
			_wait(&fileSrcLock);			// 互斥访问资源
			readOrWrite = 1;

    
   		printf_color(prompts[0],p_proc_ready->pid+9,p_proc_ready->p_name,prompts[4]);
			// // 读
		printf_color(prompts[1],p_proc_ready->pid+9,p_proc_ready->p_name,prompts[4]);
		milli_delay(p_proc_ready->ticks*1000/HZ);
		printf_color(prompts[2],p_proc_ready->pid+9,p_proc_ready->p_name,prompts[4]);

	

		_wait(&writeCntChangeLock);
			if(!(--writeCnt))
				_signal(&mutexLock);	// 最后一个写者释放
		_signal(&writeCntChangeLock);
		milli_delay(100);
	}
}

/**
 * @brief 来鹅城只办三件事
 * 
 */
void rwr(){
	while(1){
		_wait(&readCntLock);
		_wait(&readWriteLock);
			_wait(&readCntChangeLock);	// 保证互斥修改人数
				if(!(readCnt++))
					_wait(&fileSrcLock);    // 允许同时多人读, 只有第一个读者需要申请资源
			_signal(&readCntChangeLock); 
		_signal(&readWriteLock);
			readOrWrite = 0;

			printf_color(prompts[0],p_proc_ready->pid+9,p_proc_ready->p_name,prompts[3]);
			// // 读
			printf_color(prompts[1],p_proc_ready->pid+9,p_proc_ready->p_name,prompts[3]);
			milli_delay(p_proc_ready->ticks*1000/HZ);
			printf_color(prompts[2],p_proc_ready->pid+9,p_proc_ready->p_name,prompts[3]);
			_wait(&readCntChangeLock);
				if(!(--readCnt))
					_signal(&fileSrcLock);
			_signal(&readCntChangeLock);
		_signal(&readCntLock);
		milli_delay(100);
	}
}
void rww(){
	while(1){
		_wait(&readWriteLock);
		_wait(&fileSrcLock); 	

			readOrWrite = 1;

     		printf_color(prompts[0],p_proc_ready->pid+9,p_proc_ready->p_name,prompts[4]);
			// // 读
			printf_color(prompts[1],p_proc_ready->pid+9,p_proc_ready->p_name,prompts[4]);
			milli_delay(p_proc_ready->ticks*1000/HZ);
			printf_color(prompts[2],p_proc_ready->pid+9,p_proc_ready->p_name,prompts[4]);
		_signal(&fileSrcLock);
		_signal(&readWriteLock);
		milli_delay(100);
	}
}
// 空, 因为就绪列表可能为空....这时就出错了, 所以加一个空的进程哈哈哈
void E(){
	while(1){

	}
}

/**
 * @brief 清屏以及打印读写数目
 * 
 */
void F(){
while(1){
	clear();
    sleep(500*ROUND);
}
}
// /*======================================================================*
//                                TestA
//  *======================================================================*/
// void TestA()
// {
// 	int i = 0;
// 	while (1) {
// 		/* disp_str("A."); */
// 		const char a[]={"A."};
// 		printf(a);
// 		sleep(1000);
// 	}
// }

// /*======================================================================*
//                                TestB
//  *======================================================================*/
// void TestB()
// {
// 	int i = 0x1000;
// 	while(1){
// 		/* disp_str("B."); */
// 		const char b[] = {"B."};
// 		printf(b);
// 		sleep(1000);
// 	}
// }

// /*======================================================================*
//                                TestB
//  *======================================================================*/
// void TestC()
// {
// 	int i = 0x2000;
// 	while(1){
// 		printf("C.");
// 		milli_delay(1000);
// 	}
// }
