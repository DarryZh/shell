#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "shell.h"

typedef signed long  sh_size_t;
extern int sh_exec(char* cmd, sh_size_t length);

int main(void)
{
	shell_system_init();
	sh_exec("test_shell", strlen("test_shell"));
}

void test_shell(void)
{	
	printf("test shell succ!\n");
}
SHELL_CMD_EXPORT(test_shell, test_shell)
