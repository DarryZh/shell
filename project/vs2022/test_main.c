#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "shell.h"

typedef signed long  sh_size_t;
extern int sh_exec(char* cmd, sh_size_t length);

int __get_char(void)
{
	char c;
	scanf_s("%c", &c, 1);
	return (int)c;
}

int main(void)
{
	shell_system_init();
	sh_exec("test_shell", strlen("test_shell"));
	while (1) 
	{
		shell_task_entry();
	}
}	

void test_shell(void)
{	
	printf("test shell succ!\n");
}
SHELL_CMD_EXPORT(test_shell, test_shell)
