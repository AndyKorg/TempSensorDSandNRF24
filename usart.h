#ifndef USARTH_H_
#define USARTH_H_

#include <stdbool.h>
#include <stdlib.h>

#define UART_BUF_LEN	128

#define MAX_DATA_CMD	8	//maximum del data cmd
/*
	Format command:
	byte 1 - commande code
	byte 2..MAX_DATA_CMD - data command
	the last byte must be set to 0x0
*/
typedef enum command_t{
	CMD_EMPTY	= 0,			//TODO:check for delete
} command_t;

typedef struct 
{
	command_t	cmd;
	uint8_t		data[MAX_DATA_CMD];
} cmd_t;

typedef bool (*usart_cmd_cb)(cmd_t cmd);
bool usart_init(usart_cmd_cb func);
#ifdef CONSOLE_DEBUG
bool usart_is_busy(void);
#endif

#endif /* USARTH_H_ */