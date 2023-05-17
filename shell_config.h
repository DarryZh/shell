#ifndef __SHELL_CONFIG_H__
#define __SHELL_CONFIG_H__

#define RT_ALIGN_SIZE	8

#define RT_USING_FINSH

#define RT_USING_CONSOLE

#define RT_CONSOLEBUF_SIZE	128

#define FINSH_USING_SYMTAB

#define FINSH_USING_HISTORY

#define FINSH_USING_AUTH

#define RT_ASSERT(EX)  

#define rt_strlen   strlen

#define rt_strncpy  strncpy

#define rt_kprintf(...)  printf(__VA_ARGS__)

#define rt_strncmp  strncmp

#define rt_memset  memset

#define rt_memcpy   memcpy

#define rt_memmove   memmove

#endif
