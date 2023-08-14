#include <string.h>
#include <stdio.h>

#include "shell_config.h"

#ifdef SH_USING_FINSH

#include "shell.h"
#include "msh.h"

#ifdef DFS_USING_POSIX
#include <unistd.h>
#include <fcntl.h>
#endif /* DFS_USING_POSIX */

#if defined (__clang__)
#   pragma  clang diagnostic ignored "-Wincompatible-pointer-types"
#   pragma  clang diagnostic ignored "-Wint-conversion"
#   pragma  clang diagnostic ignored "-Wformat"
#endif

/* finsh thread */
#ifndef SH_USING_HEAP
    // static struct sh_thread finsh_thread;    //TODO
    sh_align(SH_ALIGN_SIZE)
    static char finsh_thread_stack[FINSH_THREAD_STACK_SIZE];
    struct finsh_shell _shell;
#endif

/* finsh symtab */
#ifdef FINSH_USING_SYMTAB
    struct finsh_syscall *_syscall_table_begin  = NULL;
    struct finsh_syscall *_syscall_table_end    = NULL;
#endif

struct finsh_shell *shell;
static char *finsh_prompt_custom = SH_NULL;


#if defined(_MSC_VER) || (defined(__GNUC__) && defined(__x86_64__))
struct finsh_syscall *finsh_syscall_next(struct finsh_syscall *call)
{
    unsigned int *ptr;
    ptr = (unsigned int *)(call + 1);
    while ((*ptr == 0) && ((unsigned int *)ptr < (unsigned int *) _syscall_table_end))
        ptr ++;

    return (struct finsh_syscall *)ptr;
}

#endif /* defined(_MSC_VER) || (defined(__GNUC__) && defined(__x86_64__)) */

#ifdef SH_USING_HEAP
int finsh_set_prompt(const char *prompt)
{
    if (finsh_prompt_custom)
    {
        sh_free(finsh_prompt_custom);
        finsh_prompt_custom = SH_NULL;
    }

    /* strdup */
    if (prompt)
    {
        finsh_prompt_custom = (char *)sh_malloc(strlen(prompt) + 1);
        if (finsh_prompt_custom)
        {
            strcpy(finsh_prompt_custom, prompt);
        }
    }

    return 0;
}
#endif /* SH_USING_HEAP */

#define _SHELL_PROMPT ""

const char *finsh_get_prompt(void)
{
    static char finsh_prompt[SH_CONSOLEBUF_SIZE + 1] = {0};

    /* check prompt mode */
    if (!shell->prompt_mode)
    {
        finsh_prompt[0] = '\0';
        return finsh_prompt;
    }

    if (finsh_prompt_custom)
    {
        strncpy(finsh_prompt, finsh_prompt_custom, sizeof(finsh_prompt) - 1);
    }
    else
    {
        strcpy(finsh_prompt, _SHELL_PROMPT);
    }

#if defined(DFS_USING_POSIX) && defined(DFS_USING_WORKDIR)
    /* get current working directory */
    getcwd(&finsh_prompt[sh_strlen(finsh_prompt)], SH_CONSOLEBUF_SIZE - sh_strlen(finsh_prompt));
#endif

    strcat(finsh_prompt, "$ ");

    return finsh_prompt;
}

/**
 * @ingroup finsh
 *
 * This function get the prompt mode of finsh shell.
 *
 * @return prompt the prompt mode, 0 disable prompt mode, other values enable prompt mode.
 */
sh_uint32_t finsh_get_prompt_mode(void)
{
    SH_ASSERT(shell != SH_NULL);
    return shell->prompt_mode;
}

/**
 * @ingroup finsh
 *
 * This function set the prompt mode of finsh shell.
 *
 * The parameter 0 disable prompt mode, other values enable prompt mode.
 *
 * @param prompt_mode the prompt mode
 */
void finsh_set_prompt_mode(sh_uint32_t prompt_mode)
{
    SH_ASSERT(shell != SH_NULL);
    shell->prompt_mode = prompt_mode;
}

int finsh_getchar(void)
{
#ifdef SH_USING_DEVICE
    char ch = 0;
#ifdef SH_USING_POSIX_STDIO
    if(read(STDIN_FILENO, &ch, 1) > 0)
    {
        return ch;
    }
    else
    {
        return -1; /* EOF */
    }
#else
    // sh_device_t device;

    // SH_ASSERT(shell != SH_NULL);

    // device = shell->device;
    // if (device == SH_NULL)
    // {
    //     return -1; /* EOF */
    // }

    // while (sh_device_read(device, -1, &ch, 1) != 1)
    // {
    //     sh_sem_take(&shell->rx_sem, SH_WAITING_FOREVER);
    //     if (shell->device != device)
    //     {
    //         device = shell->device;
    //         if (device == SH_NULL)
    //         {
    //             return -1;
    //         }
    //     }
    // }
    return ch;
#endif /* SH_USING_POSIX_STDIO */
#else
    extern int __shell_getchar(void);
    return __shell_getchar();
#endif /* SH_USING_DEVICE */
}

#if !defined(SH_USING_POSIX_STDIO) && defined(SH_USING_DEVICE)
static sh_err_t finsh_rx_ind(sh_device_t dev, sh_size_t size)
{
    SH_ASSERT(shell != SH_NULL);

    /* release semaphore to let finsh thread rx data */
    sh_sem_release(&shell->rx_sem);

    return SH_EOK;
}

/**
 * @ingroup finsh
 *
 * This function sets the input device of finsh shell.
 *
 * @param device_name the name of new input device.
 */
void finsh_set_device(const char *device_name)
{

}

/**
 * @ingroup finsh
 *
 * This function returns current finsh shell input device.
 *
 * @return the finsh shell input device name is returned.
 */
const char *finsh_get_device()
{
    SH_ASSERT(shell != SH_NULL);
    return NULL;
}
#endif /* !defined(SH_USING_POSIX_STDIO) && defined(SH_USING_DEVICE) */

/**
 * @ingroup finsh
 *
 * This function set the echo mode of finsh shell.
 *
 * FINSH_OPTION_ECHO=0x01 is echo mode, other values are none-echo mode.
 *
 * @param echo the echo mode
 */
void finsh_set_echo(sh_uint32_t echo)
{
    SH_ASSERT(shell != SH_NULL);
    shell->echo_mode = (sh_uint8_t)echo;
}

/**
 * @ingroup finsh
 *
 * This function gets the echo mode of finsh shell.
 *
 * @return the echo mode
 */
sh_uint32_t finsh_get_echo()
{
    SH_ASSERT(shell != SH_NULL);

    return shell->echo_mode;
}

#ifdef FINSH_USING_AUTH
/**
 * set a new password for finsh
 *
 * @param password new password
 *
 * @return result, SH_EOK on OK, -SH_ERROR on the new password length is less than
 *  FINSH_PASSWORD_MIN or greater than FINSH_PASSWORD_MAX
 */
sh_err_t finsh_set_password(const char *password)
{
    sh_base_t level;
    sh_size_t pw_len = sh_strlen(password);

    if (pw_len < FINSH_PASSWORD_MIN || pw_len > FINSH_PASSWORD_MAX)
        return -SH_ERROR;

    // level = sh_hw_interrupt_disable(); //TODO
    sh_strncpy(shell->password, password, FINSH_PASSWORD_MAX);
    // sh_hw_interrupt_enable(level);   //TODO

    return SH_EOK;
}

/**
 * get the finsh password
 *
 * @return password
 */
const char *finsh_get_password(void)
{
    return shell->password;
}

static void finsh_wait_auth(void)
{
    int ch;
    sh_bool_t input_finish = SH_FALSE;
    char password[FINSH_PASSWORD_MAX] = { 0 };
    sh_size_t cur_pos = 0;
    /* password not set */
    if (sh_strlen(finsh_get_password()) == 0) return;
    while (1)
    {
        sh_kprintf("Password for login: ");
        while (!input_finish)
        {
            while (1)
            {
                /* read one character from device */
                ch = (int)finsh_getchar();
                if (ch < 0)
                {
                    continue;
                }

                if (ch >= ' ' && ch <= '~' && cur_pos < FINSH_PASSWORD_MAX)
                {
                    /* change the printable characters to '*' */
                    sh_kprintf("*");
                    password[cur_pos++] = ch;
                }
                else if (ch == '\b' && cur_pos > 0)
                {
                    /* backspace */
                    cur_pos--;
                    password[cur_pos] = '\0';
                    sh_kprintf("\b \b");
                }
                else if (ch == '\r' || ch == '\n')
                {
                    sh_kprintf("\n");
                    input_finish = SH_TRUE;
                    break;
                }
            }
        }
        if (!sh_strncmp(shell->password, password, FINSH_PASSWORD_MAX)) return;
        else
        {
            /* authentication failed, delay 2S for retry */
            // sh_thread_delay(2 * SH_TICK_PER_SECOND);    //TODO
            sh_kprintf("Sorry, try again.\n");
            cur_pos = 0;
            input_finish = SH_FALSE;
            sh_memset(password, '\0', FINSH_PASSWORD_MAX);
        }
    }
}
#endif /* FINSH_USING_AUTH */

static void shell_auto_complete(char *prefix)
{
    sh_kprintf("\n");
    msh_auto_complete(prefix);

    sh_kprintf("%s%s", FINSH_PROMPT, prefix);
}

#ifdef FINSH_USING_HISTORY
static sh_bool_t shell_handle_history(struct finsh_shell *shell)
{
#if defined(_WIN32)
    int i;
    sh_kprintf("\r");

    for (i = 0; i <= 60; i++)
        putchar(' ');
    sh_kprintf("\r");

#else
    sh_kprintf("\033[2K\r");
#endif
    sh_kprintf("%s%s", FINSH_PROMPT, shell->line);
    return SH_FALSE;
}

static void shell_push_history(struct finsh_shell *shell)
{
    if (shell->line_position != 0)
    {
        /* push history */
        if (shell->history_count >= FINSH_HISTORY_LINES)
        {
            /* if current cmd is same as last cmd, don't push */
            if (memcmp(&shell->cmd_history[FINSH_HISTORY_LINES - 1], shell->line, FINSH_CMD_SIZE))
            {
                /* move history */
                int index;
                for (index = 0; index < FINSH_HISTORY_LINES - 1; index ++)
                {
                    sh_memcpy(&shell->cmd_history[index][0],
                           &shell->cmd_history[index + 1][0], FINSH_CMD_SIZE);
                }
                sh_memset(&shell->cmd_history[index][0], 0, FINSH_CMD_SIZE);
                sh_memcpy(&shell->cmd_history[index][0], shell->line, shell->line_position);

                /* it's the maximum history */
                shell->history_count = FINSH_HISTORY_LINES;
            }
        }
        else
        {
            /* if current cmd is same as last cmd, don't push */
            if (shell->history_count == 0 || memcmp(&shell->cmd_history[shell->history_count - 1], shell->line, FINSH_CMD_SIZE))
            {
                shell->current_history = shell->history_count;
                sh_memset(&shell->cmd_history[shell->history_count][0], 0, FINSH_CMD_SIZE);
                sh_memcpy(&shell->cmd_history[shell->history_count][0], shell->line, shell->line_position);

                /* increase count and set current history position */
                shell->history_count ++;
            }
        }
    }
    shell->current_history = shell->history_count;
}
#endif  /* FINSH_USING_HISTORY */

void shell_thread_entry(void *parameter)
{
    int ch;

    /* normal is echo mode */
#ifndef FINSH_ECHO_DISABLE_DEFAULT
    shell->echo_mode = 1;
#else
    shell->echo_mode = 0;
#endif

#if !defined(SH_USING_POSIX_STDIO) && defined(SH_USING_DEVICE)
    /* set console device as shell device */
    if (shell->device == SH_NULL)
    {
        sh_device_t console = sh_console_get_device();
        if (console)
        {
            finsh_set_device(console->parent.name);
        }
    }
#endif /* !defined(SH_USING_POSIX_STDIO) && defined(SH_USING_DEVICE) */

#ifdef FINSH_USING_AUTH
    /* set the default password when the password isn't setting */
    if (sh_strlen(finsh_get_password()) == 0)
    {
        if (finsh_set_password(FINSH_DEFAULT_PASSWORD) != SH_EOK)
        {
            sh_kprintf("Finsh password set failed.\n");
        }
    }
    /* waiting authenticate success */
    finsh_wait_auth();
#endif

    sh_kprintf(FINSH_PROMPT);

    while (1)
    {
        ch = (int)finsh_getchar();
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
#ifdef FINSH_USING_HISTORY
                /* prev history */
                if (shell->current_history > 0)
                    shell->current_history --;
                else
                {
                    shell->current_history = 0;
                    continue;
                }

                /* copy the history command */
                sh_memcpy(shell->line, &shell->cmd_history[shell->current_history][0],
                       FINSH_CMD_SIZE);
                shell->line_curpos = shell->line_position = (sh_uint16_t)strlen(shell->line);
                shell_handle_history(shell);
#endif
                continue;
            }
            else if (ch == 0x42) /* down key */
            {
#ifdef FINSH_USING_HISTORY
                /* next history */
                if (shell->current_history < shell->history_count - 1)
                    shell->current_history ++;
                else
                {
                    /* set to the end of history */
                    if (shell->history_count != 0)
                        shell->current_history = shell->history_count - 1;
                    else
                        continue;
                }

                sh_memcpy(shell->line, &shell->cmd_history[shell->current_history][0],
                       FINSH_CMD_SIZE);
                shell->line_curpos = shell->line_position = (sh_uint16_t)strlen(shell->line);
                shell_handle_history(shell);
#endif
                continue;
            }
            else if (ch == 0x44) /* left key */
            {
                if (shell->line_curpos)
                {
                    sh_kprintf("\b");
                    shell->line_curpos --;
                }

                continue;
            }
            else if (ch == 0x43) /* right key */
            {
                if (shell->line_curpos < shell->line_position)
                {
                    sh_kprintf("%c", shell->line[shell->line_curpos]);
                    shell->line_curpos ++;
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
                sh_kprintf("\b");

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

                sh_memmove(&shell->line[shell->line_curpos],
                           &shell->line[shell->line_curpos + 1],
                           shell->line_position - shell->line_curpos);
                shell->line[shell->line_position] = 0;

                sh_kprintf("\b%s  \b", &shell->line[shell->line_curpos]);

                /* move the cursor to the origin position */
                for (i = shell->line_curpos; i <= shell->line_position; i++)
                    sh_kprintf("\b");
            }
            else
            {
                sh_kprintf("\b \b");
                shell->line[shell->line_position] = 0;
            }

            continue;
        }

        /* handle end of line, break */
        if (ch == '\r' || ch == '\n')
        {
#ifdef FINSH_USING_HISTORY
            shell_push_history(shell);
#endif
            if (shell->echo_mode)
                sh_kprintf("\n");
            msh_exec(shell->line, shell->line_position);

            sh_kprintf(FINSH_PROMPT);
            sh_memset(shell->line, 0, sizeof(shell->line));
            shell->line_curpos = shell->line_position = 0;
            continue;
        }

        /* it's a large line, discard it */
        if (shell->line_position >= FINSH_CMD_SIZE)
            shell->line_position = 0;

        /* normal character */
        if (shell->line_curpos < shell->line_position)
        {
            int i;

            sh_memmove(&shell->line[shell->line_curpos + 1],
                       &shell->line[shell->line_curpos],
                       shell->line_position - shell->line_curpos);
            shell->line[shell->line_curpos] = ch;
            if (shell->echo_mode)
                sh_kprintf("%s", &shell->line[shell->line_curpos]);

            /* move the cursor to new position */
            for (i = shell->line_curpos; i < shell->line_position; i++)
                sh_kprintf("\b");
        }
        else
        {
            shell->line[shell->line_position] = ch;
            if (shell->echo_mode)
                sh_kprintf("%c", ch);
        }

        ch = 0;
        shell->line_position ++;
        shell->line_curpos++;
        if (shell->line_position >= FINSH_CMD_SIZE)
        {
            /* clear command line */
            shell->line_position = 0;
            shell->line_curpos = 0;
        }
    } /* end of device read */
}

void finsh_system_function_init(const void *begin, const void *end)
{
    _syscall_table_begin = (struct finsh_syscall *) begin;
    _syscall_table_end = (struct finsh_syscall *) end;
}

#if defined(__ICCARM__) || defined(__ICCRX__)               /* for IAR compiler */
#ifdef FINSH_USING_SYMTAB
    #pragma section="FSymTab"
#endif
#elif defined(_MSC_VER)
#pragma section("FSymTab$a", read)
const char __fsym_begin_name[] = "__start";
const char __fsym_begin_desc[] = "begin of finsh";
__declspec(allocate("FSymTab$a")) const struct finsh_syscall __fsym_begin =
{
    __fsym_begin_name,
    __fsym_begin_desc,
    NULL
};

#pragma section("FSymTab$z", read)
const char __fsym_end_name[] = "__end";
const char __fsym_end_desc[] = "end of finsh";
__declspec(allocate("FSymTab$z")) const struct finsh_syscall __fsym_end =
{
    __fsym_end_name,
    __fsym_end_desc,
    NULL
};
#endif

/*
 * @ingroup finsh
 *
 * This function will initialize finsh shell
 */
int shell_system_init(void)
{
    // sh_err_t result = SH_EOK;
    // sh_thread_t tid;

#ifdef FINSH_USING_SYMTAB
#ifdef __ARMCC_VERSION  /* ARM C Compiler */
    extern const int FSymTab$$Base;
    extern const int FSymTab$$Limit;
    finsh_system_function_init(&FSymTab$$Base, &FSymTab$$Limit);
#elif defined (__ICCARM__) || defined(__ICCRX__)      /* for IAR Compiler */
    finsh_system_function_init(__section_begin("FSymTab"),
                               __section_end("FSymTab"));
#elif defined (__GNUC__)
    /* GNU GCC Compiler and TI CCS */
    extern const int __fsymtab_start;
    extern const int __fsymtab_end;
    finsh_system_function_init(&__fsymtab_start, &__fsymtab_end);
#elif defined(_MSC_VER)
    unsigned int *ptr_begin, *ptr_end;

    if (shell)
    {
        sh_kprintf("finsh shell already init.\n");
        return SH_EOK;
    }

    ptr_begin = (unsigned int *)&__fsym_begin;
    ptr_begin += (sizeof(struct finsh_syscall) / sizeof(unsigned int));
    while (*ptr_begin == 0) ptr_begin ++;

    ptr_end = (unsigned int *) &__fsym_end;
    ptr_end --;
    while (*ptr_end == 0) ptr_end --;

    finsh_system_function_init(ptr_begin, ptr_end);
#endif
#endif

#ifdef SH_USING_HEAP
    // /* create or set shell structure */
    // shell = (struct finsh_shell *)sh_calloc(1, sizeof(struct finsh_shell));
    // if (shell == SH_NULL)
    // {
    //     sh_kprintf("no memory for shell\n");
    //     return -1;
    // }
    // tid = sh_thread_create(FINSH_THREAD_NAME,
    //                        finsh_thread_entry, SH_NULL,
    //                        FINSH_THREAD_STACK_SIZE, FINSH_THREAD_PRIORITY, 10);
#else
    shell = &_shell;
    // tid = &finsh_thread;
    // result = sh_thread_init(&finsh_thread,
    //                         FINSH_THREAD_NAME,
    //                         finsh_thread_entry, SH_NULL,
    //                         &finsh_thread_stack[0], sizeof(finsh_thread_stack),
    //                         FINSH_THREAD_PRIORITY, 10);
#endif /* SH_USING_HEAP */

    // sh_sem_init(&(shell->rx_sem), "shrx", 0, 0); //TODO
    finsh_set_prompt_mode(1);

    // if (tid != NULL && result == SH_EOK) //TODO
    //     sh_thread_startup(tid);  
    return 0;
}

#endif /* SH_USING_FINSH */
