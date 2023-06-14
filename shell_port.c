#include <stdio.h>

int rt_hw_console_getchar(void)
{
    int word;
    word = getc(stdin);//等待用户输入或从缓存中读一个字符
    return word;
}