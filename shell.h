#ifndef __SHELL_H__
#define __SHELL_H__

typedef long (*syscall_func)(void);

#ifdef _MSC_VER
#pragma section("FSymTab$f",read)
#endif /* _MSC_VER */

#ifdef _MSC_VER
#define SHELL_FUNCTION_EXPORT_CMD(name, cmd, desc)      \
                const char __fsym_##cmd##_name[] = #cmd;            \
                __declspec(allocate("FSymTab$f"))                   \
                const struct sh_syscall __fsym_##cmd =           \
                {                           \
                    __fsym_##cmd##_name,    \
                    (syscall_func)&name     \
                };
#pragma comment(linker, "/merge:FSymTab=mytext")

#else
#define SHELL_FUNCTION_EXPORT_CMD(name, cmd, desc)                                  \
                const char __fsym_##cmd##_name[] __attribute__((section(".rodata.name"))) = #cmd;    \
                __attribute__((used)) const struct sh_syscall __fsym_##cmd __attribute__((section("FSymTab")))= \
                {                           \
                    __fsym_##cmd##_name,    \
                    (syscall_func)&name     \
                };
#endif

#define SHELL_FUNCTION_EXPORT(name, desc)

#define SHELL_FUNCTION_EXPORT_ALIAS(name, alias, desc)

#define SHELL_CMD_EXPORT(command, desc)   \
    SHELL_FUNCTION_EXPORT_CMD(command, command, desc)

#define SHELL_CMD_EXPORT_ALIAS(command, alias, desc)  \
    SHELL_FUNCTION_EXPORT_CMD(command, alias, desc)

/* system call table */
struct sh_syscall
{
    const char     *name;       /* the name of system call */

    syscall_func func;      /* the function address of system call */
};

/* system call item */
struct sh_syscall_item
{
    struct sh_syscall_item *next;    /* next item */
    struct sh_syscall syscall;       /* syscall */
};

extern struct sh_syscall_item *global_syscall_list;
extern struct sh_syscall *_syscall_table_begin, *_syscall_table_end;

#if defined(_MSC_VER) || (defined(__GNUC__) && defined(__x86_64__))
    struct sh_syscall* sh_syscall_next(struct sh_syscall* call);
    #define SH_NEXT_SYSCALL(index)  index=sh_syscall_next(index)
#else
    #define SH_NEXT_SYSCALL(index)  index++
#endif

/* find out system call, which should be implemented in user program */
struct sh_syscall *sh_syscall_lookup(const char *name);

int shell_system_init(void);

#endif
