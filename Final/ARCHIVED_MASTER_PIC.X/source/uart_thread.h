#ifndef __my_uart_h
#define __my_uart_h

#include "messages.h"

#define MAXUARTBUF 22
#if (MAXUARTBUF > MSGLEN)
//#define MAXUARTBUF MSGLEN
#endif
typedef struct __uart_comm {
    unsigned char buffer[MAXUARTBUF];
    unsigned char buflen;
} uart_comm;

void init_uart_recv(uart_comm *);
void uart_write(unsigned char length, unsigned char *msg);
void parseUART();
#endif
