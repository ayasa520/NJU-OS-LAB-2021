
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               tty.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
                                                    Rikka     , 2021
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*  */
#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

#define TTY_FIRST (tty_table)
#define TTY_END (tty_table + NR_CONSOLES)

PRIVATE void init_tty(TTY* p_tty);
PRIVATE void tty_do_read(TTY* p_tty);
PRIVATE void tty_do_write(TTY* p_tty);
PRIVATE void put_key(TTY* p_tty, u32 key);
PRIVATE void init_control_stack(TTY* p_tty);
PRIVATE unsigned int MODE = NORMAL;
// 撤回用的
typedef struct s_control_stack{
    char control_stack[1024];
    char alpha_stack[1024]; 
    int tail;
}CONTROL_STACK;
PRIVATE CONTROL_STACK control_stack;

/*======================================================================*
                           task_tty
 *======================================================================*/

PRIVATE void init_control_stack(TTY* p_tty){
    control_stack.tail=0;
    memset(control_stack.alpha_stack,0,1024);
    memset(control_stack.control_stack,0,1024);
}

PUBLIC void task_tty() {
    TTY* p_tty;
    int tick = get_ticks();
    int cur_tick = 0;
    init_keyboard();
    init_control_stack(p_tty);
    for (p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++) {
        init_tty(p_tty);
    }
    select_console(0);
    clear();
    while (1) {
        for (p_tty = TTY_FIRST; p_tty < TTY_END; p_tty++) {
            cur_tick=sys_get_ticks();
            if((cur_tick-tick)>=30000){
                if(!(MODE^NORMAL))
                    clear();
                tick = cur_tick; 
            } 
            tty_do_read(p_tty);
            tty_do_write(p_tty);
        }
    }
}

/*======================================================================*
                           init_tty
 *======================================================================*/
PRIVATE void init_tty(TTY* p_tty) {
    p_tty->inbuf_count = 0;
    p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;

    init_screen(p_tty);
}
/**
 * @brief 记录操作步骤
 *
 * @param p_tty 没用, 但可以有用
 * @param ch
 *
 * @return 
 */
PRIVATE void add_step(TTY*p_tty,char ch){
    // 如果是退格
    if(ch=='\b'){
		char before = get_front_char(p_tty->p_console);
		if(!before){
			return;
		}
        control_stack.control_stack[control_stack.tail] = 0; // 退
        control_stack.alpha_stack[control_stack.tail++] = before; // 被退的字符
    }else {
        // 是新字符
        /* printf("%x",control_stack.tail); */
        control_stack.control_stack[control_stack.tail] = 1;
        control_stack.alpha_stack[control_stack.tail++] = ch;
    }
}

/**
 * @brief 撤回 感觉写在这儿不太合适, 这样我每次只能做一个字符, 不然就把 tty
 * 缓冲区满了
 *
 * @param p_tty 没用, 但本可以有用, 只是懒
 *
 * @return 
 */
PRIVATE void recover_step(TTY* p_tty){
    if(control_stack.tail<=0)
    {
        control_stack.tail=0;
        return;
    }   
    char action = control_stack.control_stack[--control_stack.tail];
    /* printf("tail=%x action=%x ",control_stack.tail,action); */

    char ch;
    switch(action){
        case 0:
            ch = control_stack.alpha_stack[control_stack.tail]; // 撤回一个退格操作
            /* printf("recover alpha %c:  ",ch); */
            put_key(p_tty, ch);
        break;
        case 1:
        // 撤回一个新字符操作
            put_key(p_tty,'\b' );
        break;
    }
}
/*======================================================================*
                                in_process
 *======================================================================*/
PUBLIC void in_process(TTY* p_tty, u32 key) {
    char output[2] = {'\0', '\0'};
    int raw_code = key & MASK_RAW; // MASK_RAW 是0001 1111 1111 ,把 flag 全扔掉了, 留下纯净的key
    // 可见字符, 且不是展示搜索结果阶段, 直接放上去
    if (!(key & FLAG_EXT)&&(MODE^SEARCH_RESULT)&&!(key & FLAG_CTRL_L)&&!(key & FLAG_CTRL_L)) {
        // 如果是可见
        put_key(p_tty, key);
        if(!(MODE^NORMAL))
            add_step(p_tty,(char)key);

    } else {
        // 搜索模式回车后, 只接受 ESC
        if(!(MODE^SEARCH_RESULT) && raw_code != ESC)
            return;
        switch (raw_code) {
            case 'z':
                if(key &FLAG_CTRL_L||key & FLAG_CTRL_L)
                {
                    recover_step(p_tty);
                }
            break;
            case ESC:
                if(!(MODE^NORMAL)){
                    /* put_key(p_tty,'S'); */
                    p_tty->p_console->search_start = p_tty->p_console->cursor;
                    MODE = SEARCH;
                }else{
                    /* put_key(p_tty,'N');  */
                    MODE=NORMAL;
                    clear_serach_words(p_tty->p_console);
                }
                break;
            case ENTER:
                if(!(MODE^SEARCH)){
                    MODE = SEARCH_RESULT;
                    search(p_tty->p_console);
                }else{
                    put_key(p_tty, '\n');
                    add_step(p_tty,'\n');
                }
                break;
            case TAB:
                put_key(p_tty, '\t');
                if(!(MODE^NORMAL)) add_step(p_tty,'\t');
                break;
            case BACKSPACE:
                if(!(MODE^NORMAL)||(!(MODE^SEARCH))&&p_tty->p_console->cursor>p_tty->p_console->search_start)
                {
                    put_key(p_tty, '\b');
                    if(!(MODE^NORMAL)) add_step(p_tty,'\b');
                }
                break;
            case UP:
                if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
                    scroll_screen(p_tty->p_console, SCR_DN);
                }
                break;
            case DOWN:
                if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
                    scroll_screen(p_tty->p_console, SCR_UP);
                }
                break;
            case F1:
            case F2:
            case F3:
            case F4:
            case F5:
            case F6:
            case F7:
            case F8:
            case F9:
            case F10:
            case F11:
            case F12:
                /* Alt + F1~F12 */
                if ((key & FLAG_ALT_L) || (key & FLAG_ALT_R)) {
                    select_console(raw_code - F1);
                }
                break;
            default:
                break;
        }
    }
}

/*======================================================================*
                              put_key
*======================================================================*/
PRIVATE void put_key(TTY* p_tty, u32 key) {
    if (p_tty->inbuf_count < TTY_IN_BYTES) {
        *(p_tty->p_inbuf_head) = key;
        p_tty->p_inbuf_head++;
        if (p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES) {
            p_tty->p_inbuf_head = p_tty->in_buf;
        }
        p_tty->inbuf_count++;
    }
}

/*======================================================================*
                              tty_do_read
 *======================================================================*/
PRIVATE void tty_do_read(TTY* p_tty) {
    if (is_current_console(p_tty->p_console)) {
        keyboard_read(p_tty);
    }
}

/*======================================================================*
                              tty_do_write
 *======================================================================*/
PRIVATE void tty_do_write(TTY* p_tty) {
    if (p_tty->inbuf_count) {
        char ch = *(p_tty->p_inbuf_tail);
        p_tty->p_inbuf_tail++;
        if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES) {
            p_tty->p_inbuf_tail = p_tty->in_buf;
        }
        p_tty->inbuf_count--;
    unsigned char color = DEFAULT_CHAR_COLOR;
    if(MODE&SEARCH){
        /* color */
        color = RED_FONT;
    }
        out_char(p_tty->p_console, ch,color);
    }
}

/*======================================================================*
                              tty_write
*======================================================================*/
PUBLIC void tty_write(TTY* p_tty, char* buf, int len) {
    char* p = buf;
    int i = len;

    while (i) {
        out_char(p_tty->p_console, *p++,DEFAULT_CHAR_COLOR);
        i--;
    }
}

/*======================================================================*
                              sys_write
*======================================================================*/
PUBLIC int sys_write(char* buf, int len, PROCESS* p_proc) {
    tty_write(&tty_table[p_proc->nr_tty], buf, len);
    return 0;
}
