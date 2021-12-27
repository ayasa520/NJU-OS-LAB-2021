#include "type.h"
#include "const.h"
#include "string.h"

/*
 *  为更好地理解此函数的原理，可参考 printf 的注释部分。
 */

/*======================================================================*
                                vsprintf
 *======================================================================*/
#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )

#define va_start(ap,v)  ( ap = (va_list)&v + _INTSIZEOF(v) )
//#define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
#define va_arg(ap,t)    ( *(t *)( ap=ap + _INTSIZEOF(t), ap- _INTSIZEOF(t)) )
#define va_end(ap)      ( ap = (va_list)0 )
unsigned char hex_tab[]={'0','1','2','3','4','5','6','7',\
                         '8','9','a','b','c','d','e','f'};
void i2s(char*buf,int n){
    int count = 0;
    char tmp[10];
    memset(tmp,0,20);
    memset(buf,0,20);
    while(n>0){
        tmp[count++] = n%10+'0';
        n/=10;
    }
    if(n<0){
        tmp[count++] = '-';
    }
    for(int i =0;i<count;i++){
        buf[i]=tmp[i];
    }

}

int vsprintf(char *buf, const char *fmt, va_list args)
{
	char*	p;
	char	tmp[256];
	va_list	p_next_arg = args;

	for (p=buf;*fmt;fmt++) {
		if (*fmt != '%') {
			*p++ = *fmt;
			continue;
		}

		fmt++;

		switch (*fmt) {
        case 'd':
            i2s(tmp,va_arg(args, int));
        	strcpy(p, tmp);
            p += strlen(tmp);
            break;
		case 'x':
			itoa(tmp,va_arg(args, int));
			strcpy(p, tmp);
			p += strlen(tmp);
			break;
		case 's':
    		strcpy(tmp, (va_arg(args, char *)));
            strcpy(p,tmp);
            p += strlen(tmp);
			break;
		default:
			break;
		}
	}

	return (p - buf);
}
