#ifndef __M_SHELL_H__
#define __M_SHELL_H__

#include "shell_def.h"

int msh_exec(char *cmd, rt_size_t length);
void msh_auto_complete(char *prefix);

int msh_exec_module(const char *cmd_line, int size);
int msh_exec_script(const char *cmd_line, int size);

#endif
