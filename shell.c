#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "shell.h"


typedef signed long  sh_size_t;
typedef unsigned long sh_ssize_t;

typedef int (*cmd_function_t)(int argc, char **argv);

struct sh_syscall *_syscall_table_begin  = NULL;
struct sh_syscall *_syscall_table_end    = NULL;

static sh_bool_t shell_handle_history(struct sh_shell* shell);
static void shell_push_history(struct sh_shell* shell);

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

struct sh_shell* shell;
struct sh_shell _shell;

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
     //    printf("finsh shell already init.\n");
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

     shell = &_shell;
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


/*todo*/


static void shell_auto_complete(char* prefix);

void shell_task_entry(void* parameter)
{
    int ch;

    /* normal is echo mode */
#ifndef SHELL_ECHO_DISABLE_DEFAULT
    shell->echo_mode = 1;
#else
    shell->echo_mode = 0;
#endif


#ifdef SH_USING_AUTH
    /* set the default password when the password isn't setting */
    if (strlen(shell_get_password()) == 0)
    {
        if (shell_set_password(SHELL_DEFAULT_PASSWORD) != 0)
        {
            printf("shell password set failed.\n");
        }
    }
    /* waiting authenticate success */
    shell_wait_auth();
#endif

    printf(_SH_PROMPT);

    while (1)
    {
        ch = (int)shell_getchar();
        if (ch < 0)
        {
            continue;
        }

        /*
         * handle control key
         * up key  : 0x1b 0x5b 0x41
         * down key: 0x1b 0x5b 0x42
         * right key:0x1b 0x5b 0x43
         * left key: 0x1b 0x5b 0x44
         */
        if (ch == 0x1b)
        {
            shell->stat = WAIT_SPEC_KEY;
            continue;
        }
        else if (shell->stat == WAIT_SPEC_KEY)
        {
            if (ch == 0x5b)
            {
                shell->stat = WAIT_FUNC_KEY;
                continue;
            }

            shell->stat = WAIT_NORMAL;
        }
        else if (shell->stat == WAIT_FUNC_KEY)
        {
            shell->stat = WAIT_NORMAL;

            if (ch == 0x41) /* up key */
            {
#ifdef SHELL_USING_HISTORY
                /* prev history */
                if (shell->current_history > 0)
                    shell->current_history--;
                else
                {
                    shell->current_history = 0;
                    continue;
                }

                /* copy the history command */
                memcpy(shell->line, &shell->cmd_history[shell->current_history][0],
                    SHELL_CMD_SIZE);
                shell->line_curpos = shell->line_position = (sh_uint16_t)strlen(shell->line);
                shell_handle_history(shell);
#endif
                continue;
            }
            else if (ch == 0x42) /* down key */
            {
#ifdef SHELL_USING_HISTORY
                /* next history */
                if (shell->current_history < shell->history_count - 1)
                    shell->current_history++;
                else
                {
                    /* set to the end of history */
                    if (shell->history_count != 0)
                        shell->current_history = shell->history_count - 1;
                    else
                        continue;
                }

                memcpy(shell->line, &shell->cmd_history[shell->current_history][0],
                    SHELL_CMD_SIZE);
                shell->line_curpos = shell->line_position = (sh_uint16_t)strlen(shell->line);
                shell_handle_history(shell);
#endif
                continue;
            }
            else if (ch == 0x44) /* left key */
            {
                if (shell->line_curpos)
                {
                    printf("\b");
                    shell->line_curpos--;
                }

                continue;
            }
            else if (ch == 0x43) /* right key */
            {
                if (shell->line_curpos < shell->line_position)
                {
                    printf("%c", shell->line[shell->line_curpos]);
                    shell->line_curpos++;
                }

                continue;
            }
        }

        /* received null or error */
        if (ch == '\0' || ch == 0xFF) continue;
        /* handle tab key */
        else if (ch == '\t')
        {
            int i;
            /* move the cursor to the beginning of line */
            for (i = 0; i < shell->line_curpos; i++)
                printf("\b");

            /* auto complete */
            shell_auto_complete(&shell->line[0]);
            /* re-calculate position */
            shell->line_curpos = shell->line_position = (sh_uint16_t)strlen(shell->line);

            continue;
        }
        /* handle backspace key */
        else if (ch == 0x7f || ch == 0x08)
        {
            /* note that shell->line_curpos >= 0 */
            if (shell->line_curpos == 0)
                continue;

            shell->line_position--;
            shell->line_curpos--;

            if (shell->line_position > shell->line_curpos)
            {
                int i;

                memmove(&shell->line[shell->line_curpos],
                    &shell->line[shell->line_curpos + 1],
                    shell->line_position - shell->line_curpos);
                shell->line[shell->line_position] = 0;

                printf("\b%s  \b", &shell->line[shell->line_curpos]);

                /* move the cursor to the origin position */
                for (i = shell->line_curpos; i <= shell->line_position; i++)
                    printf("\b");
            }
            else
            {
                printf("\b \b");
                shell->line[shell->line_position] = 0;
            }

            continue;
        }

        /* handle end of line, break */
        if (ch == '\r' || ch == '\n')
        {
#ifdef SHELL_USING_HISTORY
            shell_push_history(shell);
#endif
            if (shell->echo_mode)
                printf("\n");
            sh_exec(shell->line, shell->line_position);

            printf(_SH_PROMPT);
            memset(shell->line, 0, sizeof(shell->line));
            shell->line_curpos = shell->line_position = 0;
            continue;
        }

        /* it's a large line, discard it */
        if (shell->line_position >= SHELL_CMD_SIZE)
            shell->line_position = 0;

        /* normal character */
        if (shell->line_curpos < shell->line_position)
        {
            int i;

            memmove(&shell->line[shell->line_curpos + 1],
                &shell->line[shell->line_curpos],
                shell->line_position - shell->line_curpos);
            shell->line[shell->line_curpos] = ch;
            if (shell->echo_mode)
                printf("%s", &shell->line[shell->line_curpos]);

            /* move the cursor to new position */
            for (i = shell->line_curpos; i < shell->line_position; i++)
                printf("\b");
        }
        else
        {
            shell->line[shell->line_position] = ch;
            if (shell->echo_mode)
                printf("%c", ch);
        }

        ch = 0;
        shell->line_position++;
        shell->line_curpos++;
        if (shell->line_position >= SHELL_CMD_SIZE)
        {
            /* clear command line */
            shell->line_position = 0;
            shell->line_curpos = 0;
        }
    } /* end of device read */
}

const char* shell_get_prompt(void)
{
    static char shell_prompt[SHELL_CONSOLEBUF_SIZE + 1] = { 0 };

    /* check prompt mode */
    if (!shell->prompt_mode)
    {
        shell_prompt[0] = '\0';
        return shell_prompt;
    }

    strcpy(shell_prompt, _SH_PROMPT);

    strcat(shell_prompt, ">");

    return shell_prompt;
}

sh_uint32_t shell_get_prompt_mode(void)
{
    assert(shell != NULL);
    return shell->prompt_mode;
}

void shell_set_prompt_mode(sh_uint32_t prompt_mode)
{
    assert(shell != NULL);
    shell->prompt_mode = prompt_mode;
}

int shell_getchar(void) //TODO
{
    char ch = 0;

    extern int __get_char(void);

    return __get_char();
}

void shell_set_echo(sh_uint32_t echo)
{
    assert(shell != NULL);
    shell->echo_mode = (sh_uint32_t)echo;
}
sh_uint32_t shell_get_echo()
{
    assert(shell != NULL);

    return shell->echo_mode;
}

#ifdef SH_USING_AUTH

int shell_set_password(const char* password)
{
    

    return 0;
}

/**
 * get the finsh password
 *
 * @return password
 */
const char* shell_get_password(void)
{
    return shell->password;
}

static void shell_wait_auth(void)
{
    
}
#endif

static void shell_auto_complete(char* prefix)
{
    printf("\n");
    sh_auto_complete(prefix);

    printf("%s%s", _SH_PROMPT, prefix);
}

#ifdef SHELL_USING_HISTORY
static sh_bool_t shell_handle_history(struct sh_shell* shell)
{
#if defined(_WIN32)
    int i;
    printf("\r");

    for (i = 0; i <= 60; i++)
        putchar(' ');
    printf("\r");

#else
    printf("\033[2K\r");
#endif
    printf("%s%s", _SH_PROMPT, shell->line);
    return sh_false;
}

static void shell_push_history(struct sh_shell* shell)
{
    if (shell->line_position != 0)
    {
        /* push history */
        if (shell->history_count >= SHELL_HISTORY_LINES)
        {
            /* if current cmd is same as last cmd, don't push */
            if (memcmp(&shell->cmd_history[SHELL_HISTORY_LINES - 1], shell->line, SHELL_CMD_SIZE))
            {
                /* move history */
                int index;
                for (index = 0; index < SHELL_HISTORY_LINES - 1; index++)
                {
                    memcpy(&shell->cmd_history[index][0],
                        &shell->cmd_history[index + 1][0], SHELL_CMD_SIZE);
                }
                memset(&shell->cmd_history[index][0], 0, SHELL_CMD_SIZE);
                memcpy(&shell->cmd_history[index][0], shell->line, shell->line_position);

                /* it's the maximum history */
                shell->history_count = SHELL_HISTORY_LINES;
            }
        }
        else
        {
            /* if current cmd is same as last cmd, don't push */
            if (shell->history_count == 0 || memcmp(&shell->cmd_history[shell->history_count - 1], shell->line, SHELL_CMD_SIZE))
            {
                shell->current_history = shell->history_count;
                memset(&shell->cmd_history[shell->history_count][0], 0, SHELL_CMD_SIZE);
                memcpy(&shell->cmd_history[shell->history_count][0], shell->line, shell->line_position);

                /* increase count and set current history position */
                shell->history_count++;
            }
        }
    }
    shell->current_history = shell->history_count;
}
#endif
