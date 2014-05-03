#ifndef __my_uart_h
#define __my_uart_h

#include "messages.h"

#define MAXUARTBUF 4
#if (MAXUARTBUF > MSGLEN)
#define MAXUARTBUF MSGLEN
#endif
typedef struct __uart_comm {
    unsigned char buffer[MAXUARTBUF];
    unsigned char buflen;
} uart_comm;

enum { GET_MSGID, GET_COMMAND, CHECKSUM};

unsigned char State = GET_MSGID;
unsigned char buf_len = 0;
unsigned char buffer_temp[MAXUARTBUF];

unsigned char command_length = 0;
unsigned char command_count = 0;

void init_uart_recv(uart_comm *);
void uart_recv_int_handler(void);
void uart_write(unsigned char length, unsigned char *msg);
void parseUART();
#endif
