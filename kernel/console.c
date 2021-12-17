
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                              console.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
                                                    Rikka     , 2021
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*
        回车键: 把光标移到第一列
        换行键: 把光标前进到下一行
*/

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

PRIVATE void set_cursor(unsigned int position);
PRIVATE void set_video_start_addr(u32 addr);
PRIVATE void flush(CONSOLE* p_con);
PRIVATE unsigned char TABBUF[V_MEM_SIZE / 8];  //记每个 tab 开始的位置
PRIVATE unsigned char NEWLINE[V_MEM_SIZE / SCREEN_WIDTH];  // 每行有没有打换行

/*======================================================================*
                           init_screen
 *======================================================================*/
PUBLIC void init_screen(TTY* p_tty) {
    int nr_tty = p_tty - tty_table;
    p_tty->p_console = console_table + nr_tty;

    int v_mem_size = V_MEM_SIZE >> 1; /* 显存总大小 (in WORD) */

    int con_v_mem_size = v_mem_size / NR_CONSOLES;
    p_tty->p_console->original_addr = nr_tty * con_v_mem_size;
    p_tty->p_console->v_mem_limit = con_v_mem_size;
    p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;

    /* 默认光标位置在最开始处 */
    p_tty->p_console->cursor = p_tty->p_console->original_addr;

    if (nr_tty == 0) {
        /* 第一个控制台沿用原来的光标位置 */
        p_tty->p_console->cursor = disp_pos / 2;
        disp_pos = 0;
    } else {
        out_char(p_tty->p_console, nr_tty + '0', DEFAULT_CHAR_COLOR);
        out_char(p_tty->p_console, '#', DEFAULT_CHAR_COLOR);
    }
    memset(TABBUF, 0, V_MEM_SIZE / 8);
    memset(NEWLINE, 0, V_MEM_SIZE / SCREEN_WIDTH);
    set_cursor(p_tty->p_console->cursor);
}

/*======================================================================*
                           is_current_console
*======================================================================*/
PUBLIC int is_current_console(CONSOLE* p_con) {
    return (p_con == &console_table[nr_current_console]);
}

PUBLIC void clear() {
    CONSOLE* p_con = &console_table[nr_current_console];
    u8* p_vmem;
    while (p_con->cursor > p_con->original_addr) {
        p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
        p_con->cursor--;
        *(p_vmem - 2) = ' ';
        *(p_vmem - 1) = DEFAULT_CHAR_COLOR;
    }
    flush(p_con);
}

/*======================================================================*
                           out_char
 *======================================================================*/
PUBLIC void out_char(CONSOLE* p_con, char ch, unsigned char color) {
    u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);

    switch (ch) {
        case '\n':
            if (p_con->cursor <
                p_con->original_addr + p_con->v_mem_limit - SCREEN_WIDTH) {
                // 标记回车位置
                NEWLINE[(p_con->cursor-p_con->original_addr)/SCREEN_WIDTH]=(p_con->cursor-p_con->original_addr)%SCREEN_WIDTH+1;
                p_con->cursor = p_con->original_addr + SCREEN_WIDTH * ((p_con->cursor - p_con->original_addr) / SCREEN_WIDTH + 1);
                //下面这行错误! 不一定完整一行分割
                /* p_con->cursor=(p_con->cursor/SCREEN_WIDTH+1)*SCREEN_WIDTH; */
                
            }
            break;
        case '\b':
            if (p_con->cursor > p_con->original_addr) {

                int tab_index = (p_con->cursor - 1-p_con->original_addr) / 4
                    +(p_con->original_addr-(p_con->original_addr%4))/4
                    +((p_con->original_addr%4)?1:0);
                /* int tab_index = (p_con->cursor - 1-p_con->original_addr) / 4; */
                if (!TABBUF[tab_index]) {
                    p_con->cursor--;
                    *(p_vmem - 2) = ' ';
                    *(p_vmem - 1) = DEFAULT_CHAR_COLOR;
                } else {
                    // char_index 表示每四个中本来就填了几个
                    int char_index = TABBUF[tab_index] - 1;  // 为了和 0 区分, 之前在序号加了 1
                    TABBUF[tab_index] = 0;
                    p_con->cursor = (p_con->cursor-p_con->original_addr - 1) / 4 * 4 +p_con->original_addr+ char_index;
                    p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
                    for (int i = char_index; i < 4; i++) {
                        *(p_vmem + 2 * i) = ' ';
                        *(p_vmem + 2 * i + 1) = DEFAULT_CHAR_COLOR;
                    }
                }
                if( NEWLINE[(p_con->cursor-p_con->original_addr)/SCREEN_WIDTH]){
                    int pos = NEWLINE[(p_con->cursor-p_con->original_addr)/SCREEN_WIDTH]-1;
                    NEWLINE[(p_con->cursor-p_con->original_addr)/SCREEN_WIDTH]=0;
                    while((p_con->cursor-p_con->original_addr)%SCREEN_WIDTH>pos){
                        p_vmem=(u8*)(V_MEM_BASE + p_con->cursor * 2);
                        p_con->cursor--;
                        *(p_vmem - 2) = ' ';
                        *(p_vmem - 1) = DEFAULT_CHAR_COLOR;
                    }
                }
            }
            break;
        case '\t':
            // 开始我是以BASE 为基准, 但是不对啊, 我忽略的每个 tty
            //u 分的显存不一定刚好都是一整行(整四个)
            /* TABBUF[p_con->cursor / 4] = p_con->cursor % 4 + 1; */
            TABBUF[(p_con->cursor-p_con->original_addr)/4+(p_con->original_addr-(p_con->original_addr%4))/4+((p_con->original_addr%4)?1:0)]
                =(p_con->cursor-p_con->original_addr)%4+1;
            if(p_con->cursor+4-(p_con->cursor-p_con->original_addr)%4>=p_con->v_mem_limit+p_con->original_addr){

            }
            else if ((p_con->cursor - p_con->original_addr) % SCREEN_WIDTH + 4 < SCREEN_WIDTH) {
                for (int i = (p_con->cursor -p_con->original_addr)% 4; i < 4; i++) {
                    *p_vmem++ = ' ';
                    *p_vmem++ = DEFAULT_CHAR_COLOR;
                    p_con->cursor++;
                }
            } else {
                if (p_con->cursor <
                    p_con->original_addr + p_con->v_mem_limit - SCREEN_WIDTH) {
                    p_con->cursor =
                        p_con->original_addr +
                        SCREEN_WIDTH * ((p_con->cursor - p_con->original_addr) /
                                            SCREEN_WIDTH +
                                        1);
                }
            }

            break;
        default:
            if (p_con->cursor < p_con->original_addr + p_con->v_mem_limit - 1) {
                *p_vmem++ = ch;
                *p_vmem++ = color;
                p_con->cursor++;
            }
            break;
    }

    while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE) {
        scroll_screen(p_con, SCR_DN);
    }

    flush(p_con);
}

/*======================================================================*
                           flush
*======================================================================*/
PRIVATE void flush(CONSOLE* p_con) {
    if (is_current_console(p_con)) {
        set_cursor(p_con->cursor);
        // 从显存的某个位置开始显示
        set_video_start_addr(p_con->current_start_addr);
    }
}

/*======================================================================*
                            set_cursor
 *======================================================================*/
/**
 * @brief 设置光标跟随
 *
 * @param position
 *
 * @return
 */
PRIVATE void set_cursor(unsigned int position) {
    disable_int();
    out_byte(CRTC_ADDR_REG, CURSOR_H);
    out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
    out_byte(CRTC_ADDR_REG, CURSOR_L);
    out_byte(CRTC_DATA_REG, position & 0xFF);
    enable_int();
}

/*======================================================================*
                          set_video_start_addr
 *======================================================================*/
PRIVATE void set_video_start_addr(u32 addr) {
    disable_int();
    out_byte(CRTC_ADDR_REG, START_ADDR_H);
    out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
    out_byte(CRTC_ADDR_REG, START_ADDR_L);
    out_byte(CRTC_DATA_REG, addr & 0xFF);
    enable_int();
}

/**
 * @brief 查找并标红
 *
 * @param p_con
 *
 * @return 
 */
PUBLIC void search(CONSOLE*p_con){
    int content_index;
    int search_index;
    for(unsigned int i =p_con->original_addr;i<p_con->search_start;){
        _Bool find = 1;
        for( content_index=0,search_index=0;search_index<p_con->cursor-p_con->search_start;){
            // 不同的直接走
            if(i+content_index>=p_con->search_start||*((u8*)((i+content_index)*2)+V_MEM_BASE)!=*((u8*)((p_con->search_start+search_index)*2+V_MEM_BASE))){
                find = 0;
                break;
            }
            int content_tab_index = TABBUF[(i+content_index -p_con->original_addr) / 4+(p_con->original_addr-(p_con->original_addr%4))/4+((p_con->original_addr%4)?1:0)]-1;
            int search_tab_index = TABBUF[(search_index+p_con->search_start-p_con->original_addr) / 4+(p_con->original_addr-(p_con->original_addr%4))/4+((p_con->original_addr%4)?1:0)]-1;
            if(*((u8*)((i+content_index)*2+V_MEM_BASE))==' ' && *((u8*)((p_con->search_start+search_index)*2+V_MEM_BASE))==' '){
                // 表面上都是空格, 谁让我写的复杂了...处理也复杂
                if( (content_tab_index<=(i+content_index-p_con->original_addr)%4 )
                    ==(search_tab_index<=(p_con->search_start+search_index-p_con->original_addr)%4)){
                    // 都是 \t 或者都是 ' '
                    if( (content_tab_index<=(i+content_index-p_con->original_addr)%4)){
                        //都是 \t
                        content_index += 4-content_tab_index;
                        search_index += 4-search_tab_index;
                        continue;
                    }
                    else{
                        // 都是空格不管了
                    }

                }else{
                    // 一个 ' ' 一个 \t
                    find=0;
                    break;
                }
           
                
            }
            content_index+=1;
            search_index+=1;
        }
        if(find){
            for(int j=0;j<content_index;j++){
                if( *(u8*)(((i+j)*2+V_MEM_BASE))!=' ')
                    *(u8*)(((i+j)*2+1+V_MEM_BASE)) = RED_FONT;
                else
                    *(u8*)(((i+j)*2+1+V_MEM_BASE)) = RED_BACK ;
            }
            i += content_index;
        }
        else{
            i++; 
        }
    }
    
}
/*======================================================================*
                           select_console
 *======================================================================*/
PUBLIC void select_console(int nr_console) /* 0 ~ (NR_CONSOLES - 1) */
{
    if ((nr_console < 0) || (nr_console >= NR_CONSOLES)) {
        return;
    }

    nr_current_console = nr_console;

    flush(&console_table[nr_console]);
}

/*======================================================================*
                           scroll_screen
 *----------------------------------------------------------------------*
 滚屏.
 *----------------------------------------------------------------------*
 direction:
        SCR_UP	: 向上滚屏
        SCR_DN	: 向下滚屏
        其它	: 不做处理
 *======================================================================*/
PUBLIC void scroll_screen(CONSOLE* p_con, int direction) {
    if (direction == SCR_UP) {
        if (p_con->current_start_addr > p_con->original_addr) {
            p_con->current_start_addr -= SCREEN_WIDTH;
        }
    } else if (direction == SCR_DN) {
        if (p_con->current_start_addr + SCREEN_SIZE <
            p_con->original_addr + p_con->v_mem_limit) {
            p_con->current_start_addr += SCREEN_WIDTH;
        }
    } else {
    }

    flush(p_con);
}

/**
 * @brief 清除被查找的单词
 *
 * @param p_con
 *
 * @return 
 */
PUBLIC void clear_serach_words(CONSOLE* p_con) {
    u8* p_vmem;
    // 删除搜索的词
    /* while (p_con->cursor > p_con->search_start) { */
    /*     p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2); */
    /*     p_con->cursor--; */
    /*     *(p_vmem - 2) = ' '; */
    /*     *(p_vmem - 1) = DEFAULT_CHAR_COLOR; */
    /* } */
    // 具体退了几格是 out_char 内部决定, 所以决定调用几次 out_char
    for(;p_con->cursor > p_con->search_start;){
        out_char(p_con, '\b',DEFAULT_CHAR_COLOR);
    }
    // 恢复正常的颜色
    for(int i = 0;i<p_con->cursor-p_con->original_addr;i++){
        p_vmem = (u8*)(V_MEM_BASE + (p_con->original_addr+i) * 2);
        *(p_vmem+1) = DEFAULT_CHAR_COLOR;

    }
    flush(p_con);
}
PUBLIC char get_front_char(CONSOLE*p_con){
	if(p_con->cursor==p_con->original_addr){
		return 0;
	}
    char ch = *((u8*)(V_MEM_BASE + (p_con->cursor-1) * 2));
    if(ch==' '&& TABBUF[(p_con->cursor - 1-p_con->original_addr) / 4
                    +(p_con->original_addr-(p_con->original_addr%4))/4
                    +((p_con->original_addr%4)?1:0)]){
        return '\t';
    }
    return ch; 
}
