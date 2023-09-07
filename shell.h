#ifndef __SHELL_H__
#define __SHELL_H__

#include "finsh.h"
#include "shell_config.h"

#ifndef FINSH_THREAD_PRIORITY
    #define FINSH_THREAD_PRIORITY 20
#endif
#ifndef FINSH_THREAD_STACK_SIZE
    #define FINSH_THREAD_STACK_SIZE 2048
#endif
#ifndef FINSH_CMD_SIZE
    #define FINSH_CMD_SIZE      80
#endif

#define FINSH_OPTION_ECHO   0x01

#define FINSH_PROMPT        finsh_get_prompt()
const char *finsh_get_prompt(void);
int finsh_set_prompt(const char *prompt);

#ifdef FINSH_USING_HISTORY
    #ifndef FINSH_HISTORY_LINES
        #define FINSH_HISTORY_LINES 5
    #endif
#endif

#ifdef FINSH_USING_AUTH
    #ifndef FINSH_PASSWORD_MAX
        #define FINSH_PASSWORD_MAX 32
    #endif
    #ifndef FINSH_PASSWORD_MIN
        #define FINSH_PASSWORD_MIN 3
    #endif
    #ifndef FINSH_DEFAULT_PASSWORD
        #define FINSH_DEFAULT_PASSWORD "password"
    #endif
#endif /* FINSH_USING_AUTH */

#ifndef FINSH_THREAD_NAME
    #define FINSH_THREAD_NAME   "shell"
#endif

enum input_stat
{
    WAIT_NORMAL,
    WAIT_SPEC_KEY,
    WAIT_FUNC_KEY,
};
struct finsh_shell
{
    // struct sh_semaphore rx_sem;      //TODO

    enum input_stat stat;

    sh_uint8_t echo_mode: 1;
    sh_uint8_t prompt_mode: 1;

#ifdef FINSH_USING_HISTORY
    sh_uint16_t current_history;
    sh_uint16_t history_count;

    char cmd_history[FINSH_HISTORY_LINES][FINSH_CMD_SIZE];
#endif

    char line[FINSH_CMD_SIZE + 1];
    sh_uint16_t line_position;
    sh_uint16_t line_curpos;

#if !defined(SH_USING_POSIX_STDIO) && defined(SH_USING_DEVICE)
    sh_device_t device;
#endif

#ifdef FINSH_USING_AUTH
    char password[FINSH_PASSWORD_MAX];
#endif
};

void finsh_set_echo(sh_uint32_t echo);
sh_uint32_t finsh_get_echo(void);

int shell_system_init(void);
void shell_thread_entry(void *parameter);

void shell_printf(const char* fmt,...);

const char *finsh_get_device(void);
int finsh_getchar(void);

sh_uint32_t finsh_get_prompt_mode(void);
void finsh_set_prompt_mode(sh_uint32_t prompt_mode);

#ifdef FINSH_USING_AUTH
    sh_err_t finsh_set_password(const char *password);
    const char *finsh_get_password(void);
#endif

#endif