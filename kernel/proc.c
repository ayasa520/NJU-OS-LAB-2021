
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
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

/**
 * @brief 时间片轮转调度
 *  需要切换的情况： 
 *  当前进程不在就序列表:
 *       1. 请求的资源获取不能 BLOCKED
 *       2. 调用了 sleep TIMED_WAITING
 * 		 3. 完了 WAITING
 * 一个进程每次执行一个时间片, 完了就放队尾
 * 直到剩余 ticks 变成零, 移出就绪队列
*
 * @return void
 */
PUBLIC void schedule()
{
    //  for (int i = 0; i < 7; i++) {
      
	//  printf("ticks:%x pid:%x ticks:%x ticks_used:%x priority:%x state:%x\n",get_ticks(),proc_table[i].pid,proc_table[i].ticks,proc_table[i].ticks_used,proc_table[i].priority,proc_table[i].state);
    //  }
	if(p_proc_ready->state==RUNNABLE){
		if( p_proc_ready->ticks<=0)
		{
			//  ticks 用完了, 设成 WAITING, 等所有的都没了再重加
            if(p_proc_ready->pid!=6)
            {	
                p_proc_ready->state=WAITING;
                dequeue(&readyQueue);
            }
            else
                p_proc_ready->ticks = p_proc_ready->priority;
			// printf("no more ticks del%x\n",p_proc_ready->pid);
		}else if(p_proc_ready->ticks_used>=ROUND){
			p_proc_ready->ticks_used = 0;
			// printf("no more rounds del%x\n",p_proc_ready->pid);
			dequeue(&readyQueue);
			enqueue(&readyQueue,p_proc_ready);
		}
	}


	// 将时机已到的被 sleep 的进程唤醒， 加入就绪队列
	for(int i =0;i<NR_TASKS-1;i++){
		if(proc_table[i].state==TIMED_WAITING&&get_ticks()>=proc_table[i].wake_up){
			proc_table[i].state=RUNNABLE;
			proc_table[i].ticks_used=0; 
			// printf("wakeup %x\n",proc_table[i].pid);
			enqueue(&readyQueue, &proc_table[i]);
		}
	}

	// 就绪列表为空, 所有 WAITING 的进程重新加入
    // 就只有一个空的
	if(readyQueue.length<=1){
		for(int i = 0;i<NR_TASKS-1;i++){
			if(proc_table[i].state==WAITING){
				proc_table[i].state=RUNNABLE;
				proc_table[i].ticks=proc_table[i].priority;
				// printf("add:%x\n",p_proc_ready->pid);
				proc_table[i].ticks_used=0; 
				enqueue(&readyQueue,&proc_table[i]);
			}
		}
	}

        if(readyQueue.length<2)
        {p_proc_ready = &proc_table[NR_TASKS-1];
            printf("ookokokoko\n");
        }
        else
            p_proc_ready =getFront( &readyQueue);


	
	// PROCESS* p;
	// int	 greatest_ticks = 0;

	// while (!greatest_ticks) {
	// 	for (p = proc_table; p < proc_table+NR_TASKS; p++) {
	// 		if (p->ticks > greatest_ticks) {
	// 			greatest_ticks = p->ticks;
	// 			p_proc_ready = p;
	// 		}
	// 	}

	// 	if (!greatest_ticks) {
	// 		for (p = proc_table; p < proc_table+NR_TASKS; p++) {
	// 			p->ticks = p->priority;
	// 		}
	// 	}
	// }
}
/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}

PUBLIC void sys_print(char*s){
    disp_str(s);
}

PUBLIC void sys_wait(SEMAPHORE*s){
	// printf("before --\n");

	if(--(s->value)>=0){	return;
	}
	// 既然这个进程调用了系统调用, 那他肯定在队首
	// syscall.asm 传参也行,先试试这个
	p_proc_ready->state=BLOCKED;

	// printf("before enqueue readyCNt %x %x %x\n",&(s->length),&(readCntLock.length));
    dequeue(&readyQueue);
	enqueue(&(s->queue),p_proc_ready);
	schedule();

}
PUBLIC void sys_signal(SEMAPHORE*s){
	if(++(s->value)>0){
		// 这说明没有进程正在等待这个资源
		return;
	}
	PROCESS*p = dequeue(&(s->queue));//队首可获得资源
    p->state = RUNNABLE;
	enqueue(&readyQueue,p);
	schedule();
}
PUBLIC void sys_clear(){

		disp_pos = 0;


        const char space[] =" ";
        for (int i = 0; i < 80 * 25; ++i) {
            disp_str((char*)space);
        }
     
        disp_pos = 0;
}
