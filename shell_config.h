#ifndef __SHELL_CONFIG_H__
#define __SHELL_CONFIG_H__

#define _SH_PROMPT "$"

#define SH_ARG_MAX  6

#define SHELL_CONSOLEBUF_SIZE  128

#ifndef SHELL_CMD_SIZE
	#define SHELL_CMD_SIZE      80
#endif

#define SHELL_USING_HISTORY		
#ifdef SHELL_USING_HISTORY
	#ifndef SHELL_HISTORY_LINES
		#define SHELL_HISTORY_LINES 5
	#endif
#endif


#endif
