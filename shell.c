#include <stdio.h>
#include <string.h>
#include <assert.h>

#define SH_ARG_MAX  6

#include "shell.h"

typedef signed long  sh_size_t;
typedef unsigned long sh_ssize_t;

typedef int (*cmd_function_t)(int argc, char **argv);

struct sh_syscall *_syscall_table_begin  = NULL;
struct sh_syscall *_syscall_table_end    = NULL;

int sh_help(int argc, char **argv)
{
    printf("shell commands:\n");
    {
        struct sh_syscall *index;

        for (index = _syscall_table_begin;
                index < _syscall_table_end;
                SH_NEXT_SYSCALL(index))
        {
            printf("%s ", index->name);
        }
    }
    printf("\n");

    return 0;
}
SHELL_CMD_EXPORT_ALIAS(sh_help, help, raw help.);

static int sh_split(char *cmd, sh_size_t length, char *argv[SH_ARG_MAX])
{
    char *ptr;
    sh_size_t position;
    sh_size_t argc;
    sh_size_t i;

    ptr = cmd;
    position = 0;
    argc = 0;

    while (position < length)
    {
        /* strip bank and tab */
        while ((*ptr == ' ' || *ptr == '\t') && position < length)
        {
            *ptr = '\0';
            ptr ++;
            position ++;
        }

        if (argc >= SH_ARG_MAX)
        {
            printf("Too many args ! We only Use:\n");
            for (i = 0; i < argc; i++)
            {
                printf("%s ", argv[i]);
            }
            printf("\n");
            break;
        }

        if (position >= length) break;

        /* handle string */
        if (*ptr == '"')
        {
            ptr ++;
            position ++;
            argv[argc] = ptr;
            argc ++;

            /* skip this string */
            while (*ptr != '"' && position < length)
            {
                if (*ptr == '\\')
                {
                    if (*(ptr + 1) == '"')
                    {
                        ptr ++;
                        position ++;
                    }
                }
                ptr ++;
                position ++;
            }
            if (position >= length) break;

            /* skip '"' */
            *ptr = '\0';
            ptr ++;
            position ++;
        }
        else
        {
            argv[argc] = ptr;
            argc ++;
            while ((*ptr != ' ' && *ptr != '\t') && position < length)
            {
                ptr ++;
                position ++;
            }
            if (position >= length) break;
        }
    }

    return argc;
}

static cmd_function_t sh_get_cmd(char *cmd, int size)
{
    struct sh_syscall *index;
    cmd_function_t cmd_func = NULL;

    for (index = _syscall_table_begin;
            index < _syscall_table_end;
            SH_NEXT_SYSCALL(index))
    {
        if (strncmp(index->name, cmd, size) == 0 &&
                index->name[size] == '\0')
        {
            cmd_func = (cmd_function_t)index->func;
            break;
        }
    }

    return cmd_func;
}

static int _sh_exec_cmd(char *cmd, sh_size_t length, int *retp)
{
    int argc;
    sh_size_t cmd0_size = 0;
    cmd_function_t cmd_func;
    char *argv[SH_ARG_MAX];

    assert(cmd);
    assert(retp);

    /* find the size of first command */
    while ((cmd[cmd0_size] != ' ' && cmd[cmd0_size] != '\t') && cmd0_size < length)
        cmd0_size ++;
    if (cmd0_size == 0)
        return -1;

    cmd_func = sh_get_cmd(cmd, cmd0_size);
    if (cmd_func == NULL)
        return -1;

    /* split arguments */
    memset(argv, 0x00, sizeof(argv));
    argc = sh_split(cmd, length, argv);
    if (argc == 0)
        return -1;

    /* exec this command */
    *retp = cmd_func(argc, argv);
    return 0;
}

int sh_exec(char *cmd, sh_size_t length)
{
    int cmd_ret;

    /* strim the beginning of command */
    while ((length > 0) && (*cmd  == ' ' || *cmd == '\t'))
    {
        cmd++;
        length--;
    }

    if (length == 0)
        return 0;

    /* Exec sequence:
     * 1. built-in command
     * 2. module(if enabled)
     */
    if (_sh_exec_cmd(cmd, length, &cmd_ret) == 0)
    {
        return cmd_ret;
    }

    /* truncate the cmd at the first space. */
    {
        char *tcmd;
        tcmd = cmd;
        while (*tcmd != ' ' && *tcmd != '\0')
        {
            tcmd++;
        }
        *tcmd = '\0';
    }
    printf("%s: command not found.\n", cmd);
    return -1;
}

static int str_common(const char *str1, const char *str2)
{
    const char *str = str1;

    while ((*str != 0) && (*str2 != 0) && (*str == *str2))
    {
        str ++;
        str2 ++;
    }

    return (str - str1);
}

void sh_auto_complete(char *prefix)
{
    int length, min_length;
    const char *name_ptr, *cmd_name;
    struct sh_syscall *index;

    min_length = 0;
    name_ptr = NULL;

    if (*prefix == '\0')
    {
        sh_help(0, NULL);
        return;
    }

    /* checks in internal command */
    {
        for (index = _syscall_table_begin; index < _syscall_table_end; SH_NEXT_SYSCALL(index))
        {
            /* skip finsh shell function */
            cmd_name = (const char *) index->name;
            if (strncmp(prefix, cmd_name, strlen(prefix)) == 0)
            {
                if (min_length == 0)
                {
                    /* set name_ptr */
                    name_ptr = cmd_name;
                    /* set initial length */
                    min_length = strlen(name_ptr);
                }

                length = str_common(name_ptr, cmd_name);
                if (length < min_length)
                    min_length = length;

                printf("%s\n", cmd_name);
            }
        }
    }

    /* auto complete string */
    if (name_ptr != NULL)
    {
        strncpy(prefix, name_ptr, min_length);
    }

    return ;
}


void shell_system_function_init(const void *begin, const void *end)
{
    _syscall_table_begin = (struct sh_syscall *) begin;
    _syscall_table_end = (struct sh_syscall *) end;
}

#ifdef _MSC_VER
#pragma section("FSymTab$a", read)
const char __fsym_begin_name[] = "__start";
const char __fsym_begin_desc[] = "begin of finsh";
__declspec(allocate("FSymTab$a")) const struct sh_syscall __fsym_begin =
{
    __fsym_begin_name,
    NULL
};

#pragma section("FSymTab$z", read)
const char __fsym_end_name[] = "__end";
const char __fsym_end_desc[] = "end of finsh";
__declspec(allocate("FSymTab$z")) const struct sh_syscall __fsym_end =
{
    __fsym_end_name,
    NULL
};
#endif

int shell_system_init(void)
{
    #ifdef __ARMCC_VERSION  /* ARM C Compiler */
    extern const int FSymTab$$Base;
    extern const int FSymTab$$Limit;
    shell_system_function_init(&FSymTab$$Base, &FSymTab$$Limit);
#elif defined (__ICCARM__) || defined(__ICCRX__)      /* for IAR Compiler */
    finsh_system_function_init(__section_begin("FSymTab"),
                               __section_end("FSymTab"));
#elif defined (__GNUC__) || defined(__TI_COMPILER_VERSION__) || defined(__TASKING__)
    /* GNU GCC Compiler and TI CCS */
    extern const int __fsymtab_start;
    extern const int __fsymtab_end;
    shell_system_function_init(&__fsymtab_start, &__fsymtab_end);
#elif defined(__ADSPBLACKFIN__) /* for VisualDSP++ Compiler */
    shell_system_function_init(&__fsymtab_start, &__fsymtab_end);
#elif defined(_MSC_VER)
     unsigned int *ptr_begin, *ptr_end;

     //if (shell)
     //{
     //    rt_kprintf("finsh shell already init.\n");
     //    return RT_EOK;
     //}

     ptr_begin = (unsigned int *)&__fsym_begin;
     ptr_begin += (sizeof(struct sh_syscall) / sizeof(unsigned int));
     while (*ptr_begin == 0) ptr_begin ++;

     ptr_end = (unsigned int *) &__fsym_end;
     ptr_end --;
     while (*ptr_end == 0) ptr_end --;

     shell_system_function_init(ptr_begin, ptr_end);
#endif
    return 0;
}

#if defined(_MSC_VER) || (defined(__GNUC__) && defined(__x86_64__))
struct sh_syscall* sh_syscall_next(struct sh_syscall* call)
{
    unsigned int* ptr;
    ptr = (unsigned int*)(call + 1);
    while ((*ptr == 0) && ((unsigned int*)ptr < (unsigned int*)_syscall_table_end))
        ptr++;

    return (struct sh_syscall*)ptr;
}

#endif /* defined(_MSC_VER) || (defined(__GNUC__) && defined(__x86_64__)) */
