
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               clock.c
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
                           clock_handler
 *======================================================================*/
PUBLIC void clock_handler(int irq)
{
	ticks++;
        p_proc_ready->ticks_used++;
	p_proc_ready->ticks--;

	if (k_reenter != 0) {
		return;
	}

	// 没到一个时间片且剩余 tick 还有且可以执行, 不用调度
	if (p_proc_ready->ticks_used<ROUND &&  p_proc_ready->ticks > 0 && p_proc_ready->state==RUNNABLE) {
		return;
	}
        // 
	schedule();
}

/*======================================================================*
                              milli_delay
 *======================================================================*/
PUBLIC void milli_delay(int milli_sec)
{
        int t = get_ticks();

        while(((get_ticks() - t) * 1000 / HZ) < milli_sec) {}
}

/*======================================================================*
                           init_clock
 *======================================================================*/
PUBLIC void init_clock()
{
        /* 初始化 8253 PIT */
        out_byte(TIMER_MODE, RATE_GENERATOR);
        out_byte(TIMER0, (u8) (TIMER_FREQ/HZ) );
        out_byte(TIMER0, (u8) ((TIMER_FREQ/HZ) >> 8));

        put_irq_handler(CLOCK_IRQ, clock_handler);      /* 设定时钟中断处理程序 */
        enable_irq(CLOCK_IRQ);                        /* 让8259A可以接收时钟中断 */
}



PUBLIC void sys_sleep(int milli_sec){
	PROCESS*p_proc = p_proc_ready;
    

        if(p_proc->state==RUNNABLE)
        {
                // printf("%x",ticks);
                // for (int i = 0; i < 3; i++)
                // {
                //  if(proc_table[i].state==RUNNABLE)

		// 	printf("pid:%x ticks:%x wakeup:%x\n",
		// 	i,
		// 	proc_table[i].ticks);        /* code */
                // }
                
                p_proc->wake_up = milli_sec*HZ/1000+get_ticks();
                // printf("timewait%x tick%x now%x\n",p_proc->pid,p_proc->ticks,p_proc_ready->pid);
                p_proc->state=TIMED_WAITING;
                dequeue(&readyQueue);// 将当前的进程移出就绪队列
                schedule();//马上进行调度
        }
        // 好像没用
}


