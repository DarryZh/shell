#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "finsh.h"
#include "shell.h"

extern void finsh_thread_entry(void *parameter);

int main(void)
{
	finsh_system_init();
	while (1) 
	{
		finsh_thread_entry(NULL);
	}
	return 0;
}	

void test_shell_helloworld(void)
{	
	printf("test shell helloworld!\n");
}
MSH_CMD_EXPORT(test_shell_helloworld, test_shell_helloworld);

