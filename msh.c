#include <stdio.h>
#include <string.h>
#include "shell_config.h"

#ifdef RT_USING_FINSH

#ifndef FINSH_ARG_MAX
#define FINSH_ARG_MAX    8
#endif /* FINSH_ARG_MAX */

#include "msh.h"
#include "shell.h"
#ifdef DFS_USING_POSIX
#include <dfs_file.h>
#include <unistd.h>
#include <fcntl.h>
#endif /* DFS_USING_POSIX */

typedef int (*cmd_function_t)(int argc, char **argv);

int msh_help(int argc, char **argv)
{
    rt_kprintf("shell commands:\n");
    {
        struct finsh_syscall *index;

        for (index = _syscall_table_begin;
                index < _syscall_table_end;
                FINSH_NEXT_SYSCALL(index))
        {
#if defined(FINSH_USING_DESCRIPTION) && defined(FINSH_USING_SYMTAB)
            rt_kprintf("%-16s - %s\n", index->name, index->desc);
#else
            rt_kprintf("%s ", index->name);
#endif
        }
    }
    rt_kprintf("\n");

    return 0;
}
MSH_CMD_EXPORT_ALIAS(msh_help, help, RT-Thread shell help.);

static int msh_split(char *cmd, rt_size_t length, char *argv[FINSH_ARG_MAX])
{
    char *ptr;
    rt_size_t position;
    rt_size_t argc;
    rt_size_t i;

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

        if (argc >= FINSH_ARG_MAX)
        {
            rt_kprintf("Too many args ! We only Use:\n");
            for (i = 0; i < argc; i++)
            {
                rt_kprintf("%s ", argv[i]);
            }
            rt_kprintf("\n");
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

static cmd_function_t msh_get_cmd(char *cmd, int size)
{
    struct finsh_syscall *index;
    cmd_function_t cmd_func = RT_NULL;

    for (index = _syscall_table_begin;
            index < _syscall_table_end;
            FINSH_NEXT_SYSCALL(index))
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

static int _msh_exec_cmd(char *cmd, rt_size_t length, int *retp)
{
    int argc;
    rt_size_t cmd0_size = 0;
    cmd_function_t cmd_func;
    char *argv[FINSH_ARG_MAX];

    RT_ASSERT(cmd);
    RT_ASSERT(retp);

    /* find the size of first command */
    while ((cmd[cmd0_size] != ' ' && cmd[cmd0_size] != '\t') && cmd0_size < length)
        cmd0_size ++;
    if (cmd0_size == 0)
        return -RT_ERROR;

    cmd_func = msh_get_cmd(cmd, cmd0_size);
    if (cmd_func == RT_NULL)
        return -RT_ERROR;

    /* split arguments */
    rt_memset(argv, 0x00, sizeof(argv));
    argc = msh_split(cmd, length, argv);
    if (argc == 0)
        return -RT_ERROR;

    /* exec this command */
    *retp = cmd_func(argc, argv);
    return 0;
}

int msh_exec(char *cmd, rt_size_t length)
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
    if (_msh_exec_cmd(cmd, length, &cmd_ret) == 0)
    {
        return cmd_ret;
    }
#ifdef DFS_USING_POSIX
#ifdef DFS_USING_WORKDIR
    if (msh_exec_script(cmd, length) == 0)
    {
        return 0;
    }
#endif

#endif /* DFS_USING_POSIX */

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
    rt_kprintf("%s: command not found.\n", cmd);
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

#ifdef DFS_USING_POSIX
void msh_auto_complete_path(char *path)
{
    DIR *dir = RT_NULL;
    struct dirent *dirent = RT_NULL;
    char *full_path, *ptr, *index;

    if (!path)
        return;

    full_path = (char *)rt_malloc(256);
    if (full_path == RT_NULL) return; /* out of memory */

    if (*path != '/')
    {
        getcwd(full_path, 256);
        if (full_path[rt_strlen(full_path) - 1]  != '/')
            strcat(full_path, "/");
    }
    else *full_path = '\0';

    index = RT_NULL;
    ptr = path;
    for (;;)
    {
        if (*ptr == '/') index = ptr + 1;
        if (!*ptr) break;

        ptr ++;
    }
    if (index == RT_NULL) index = path;

    if (index != RT_NULL)
    {
        char *dest = index;

        /* fill the parent path */
        ptr = full_path;
        while (*ptr) ptr ++;

        for (index = path; index != dest;)
            *ptr++ = *index++;
        *ptr = '\0';

        dir = opendir(full_path);
        if (dir == RT_NULL) /* open directory failed! */
        {
            rt_free(full_path);
            return;
        }

        /* restore the index position */
        index = dest;
    }

    /* auto complete the file or directory name */
    if (*index == '\0') /* display all of files and directories */
    {
        for (;;)
        {
            dirent = readdir(dir);
            if (dirent == RT_NULL) break;

            rt_kprintf("%s\n", dirent->d_name);
        }
    }
    else
    {
        int multi = 0;
        rt_size_t length, min_length;

        min_length = 0;
        for (;;)
        {
            dirent = readdir(dir);
            if (dirent == RT_NULL) break;

            /* matched the prefix string */
            if (strncmp(index, dirent->d_name, rt_strlen(index)) == 0)
            {
                multi ++;
                if (min_length == 0)
                {
                    min_length = rt_strlen(dirent->d_name);
                    /* save dirent name */
                    strcpy(full_path, dirent->d_name);
                }

                length = str_common(dirent->d_name, full_path);

                if (length < min_length)
                {
                    min_length = length;
                }
            }
        }

        if (min_length)
        {
            if (multi > 1)
            {
                /* list the candidate */
                rewinddir(dir);

                for (;;)
                {
                    dirent = readdir(dir);
                    if (dirent == RT_NULL) break;

                    if (strncmp(index, dirent->d_name, rt_strlen(index)) == 0)
                        rt_kprintf("%s\n", dirent->d_name);
                }
            }

            length = index - path;
            rt_memcpy(index, full_path, min_length);
            path[length + min_length] = '\0';

            /* try to locate folder */
            if (multi == 1)
            {
                struct stat buffer = {0};
                if ((stat(path, &buffer) == 0) && (S_ISDIR(buffer.st_mode)))
                {
                    strcat(path, "/");
                }
            }
        }
    }

    closedir(dir);
    rt_free(full_path);
}
#endif /* DFS_USING_POSIX */

void msh_auto_complete(char *prefix)
{
    int length, min_length;
    const char *name_ptr, *cmd_name;
    struct finsh_syscall *index;

    min_length = 0;
    name_ptr = RT_NULL;

    if (*prefix == '\0')
    {
        msh_help(0, RT_NULL);
        return;
    }

#ifdef DFS_USING_POSIX
    /* check whether a spare in the command */
    {
        char *ptr;

        ptr = prefix + rt_strlen(prefix);
        while (ptr != prefix)
        {
            if (*ptr == ' ')
            {
                msh_auto_complete_path(ptr + 1);
                break;
            }

            ptr --;
        }
    }
#endif /* DFS_USING_POSIX */

    /* checks in internal command */
    {
        for (index = _syscall_table_begin; index < _syscall_table_end; FINSH_NEXT_SYSCALL(index))
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

                rt_kprintf("%s\n", cmd_name);
            }
        }
    }

    /* auto complete string */
    if (name_ptr != NULL)
    {
        rt_strncpy(prefix, name_ptr, min_length);
    }

    return ;
}
#endif /* RT_USING_FINSH */
