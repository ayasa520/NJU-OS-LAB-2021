#include "type.h"
#include "const.h"

void printf(const char*s,...){
    int i;
	char buf[256];

	va_list arg = (va_list)((char*)(&fmt) + 4); /*4是参数fmt所占堆栈中的大小*/
	i = vsprintf(buf, fmt, arg);
	print(buf, i);

	return i;
    
}
