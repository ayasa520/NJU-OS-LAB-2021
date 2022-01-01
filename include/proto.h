
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            proto.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* klib.asm */
PUBLIC void	out_byte(u16 port, u8 value);
PUBLIC u8	in_byte(u16 port);
PUBLIC void	disp_str(char * info);
PUBLIC void	disp_color_str(char * info, int color);

/* protect.c */
PUBLIC void	init_prot();
PUBLIC u32	seg2phys(u16 seg);

/* klib.c */
PUBLIC void	delay(int time);

/* kernel.asm */
void restart();

/* main.c */
void TestA();
void TestB();
void TestC();
void r1r();
void r1w();
void w1r();
void rwr();
void rww();
void w1w();
void F();
void G();
/* i8259.c */
PUBLIC void put_irq_handler(int irq, irq_handler handler);
PUBLIC void spurious_irq(int irq);

/* clock.c */
PUBLIC void clock_handler(int irq);
PUBLIC void init_clock();

/* keyboard.c */
PUBLIC void init_keyboard();

/* 以下是系统调用相关 */

typedef struct s_semaphore SEMAPHORE;


/* proc.c */
PUBLIC  int     sys_get_ticks();        /* sys_call */
PUBLIC void sys_print(char*s);
PUBLIC void sys_sleep(int millisec);
PUBLIC void sys_wait(SEMAPHORE*s);
PUBLIC void sys_signal(SEMAPHORE*s);
PUBLIC void sys_clear();

/* syscall.asm */
PUBLIC  void    sys_call();             /* int_handler */
PUBLIC  int     get_ticks();
PUBLIC void _wait();
PUBLIC void _signal();
PUBLIC void sleep();
PUBLIC void printf(const char*fmt,...);
PUBLIC void printf_color(const char*fmt,int color,...);
PUBLIC void print(char*s);
PUBLIC void clear();




