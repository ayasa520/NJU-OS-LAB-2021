
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef _ORANGES_CONSOLE_H_
#define _ORANGES_CONSOLE_H_


/* CONSOLE */
typedef struct s_console
{
    /* 以下几个我感觉都是相对于V_MEM_BASE 的偏移量 */
	unsigned int	current_start_addr;	/* 当前显示到了什么位置 更像是当前屏幕左上角从哪里开始, 因为滚动的时候用的就是他  */
	unsigned int	original_addr;		/* 当前控制台对应显存位置 */
	unsigned int	v_mem_limit;		/* 当前控制台占的显存大小 */
	unsigned int	cursor;			/* 当前光标位置 */
    unsigned int search_start; // 有几个 tab, original_addr/8 应该就是相对于 V_MEM_BASE 的 tab 列表开始.
}CONSOLE;

#define SCR_UP	1	/* scroll forward */
#define SCR_DN	-1	/* scroll backward */

#define SCREEN_SIZE		(80 * 25)
#define SCREEN_WIDTH		80

#define DEFAULT_CHAR_COLOR	0x07	/* 0000 0111 黑底白字 */
#define RED_FONT	0xC					/*0000 1100 黑底红字*/
#define RED_BACK  0x40 // 红色背景 
#endif /* _ORANGES_CONSOLE_H_ */
