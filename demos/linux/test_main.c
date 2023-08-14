#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "shell.h"

int main(void)
{
#ifdef SH_USING_FINSH
	shell_system_init();
#endif
	while (1) 
	{
#ifdef SH_USING_FINSH
		shell_thread_entry(NULL);
#endif
	}
	return 0;
}	

#ifdef SH_USING_FINSH
void test_shell_helloworld(uint8_t argc, char **argv)
{
	int a = 0;
    if(argc >= 2 ){
        a = atoi(argv[1]);
    } else {
        // return ;
    }	

	printf("test shell helloworld!\n");
}
SHELL_CMD_EXPORT(test_shell_helloworld, test_shell_helloworld);
#endif
