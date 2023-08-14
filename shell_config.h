#ifndef __SHELL_CONFIG_H__
#define __SHELL_CONFIG_H__

#define SH_USING_FINSH

#define FINSH_USING_SYMTAB

#define FINSH_USING_HISTORY

// #define FINSH_USING_AUTH

#define SH_ALIGN_SIZE	8

#define SH_CONSOLEBUF_SIZE	128

#define FINSH_CMD_SIZE       64

#define SH_ASSERT(EX)                       // please config user's assert  

#define sh_kprintf(...)  printf(__VA_ARGS__)            // please config user's printf

#define sh_strlen   strlen

#define sh_strncpy  strncpy

#define sh_strncmp  strncmp

#define sh_memset  memset

#define sh_memcpy   memcpy

#define sh_memmove   memmove

#endif
