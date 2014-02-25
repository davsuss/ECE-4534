#ifndef __my_uart_h
#define __my_uart_h

#include "messages.h"

#define MOTOR_ROTATIONS 4

#define MAXUARTBUF 22
#if (MAXUARTBUF > MSGLEN)
#define MAXUARTBUF MSGLEN
#endif
typedef struct __uart_comm {
    unsigned char buffer[MAXUARTBUF];
    unsigned char buflen;
} uart_comm;

enum {GET_MSG_COUNT, GET_MSG_ID_AND_LEN, GET_COMMANDS};

unsigned char STATE = GET_MSG_COUNT;
unsigned char COMMAND_LEN = 0;
unsigned char COMMAND_REC = 0;

void init_uart_recv(uart_comm *);
void uart_recv_int_handler(void);

void uart_write(unsigned char length, unsigned char *msg);

void retrieve_command();

#endif
