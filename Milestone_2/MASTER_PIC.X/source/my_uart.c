#include "maindefs.h"
#ifndef __XC8
#include <usart.h>
#else
#include <plib/usart.h>
#endif
#include "my_uart.h"

static uart_comm *uc_ptr;
typedef enum{MSGCOUNT,MSGID,COMMAND,CHECKSUM} UARTSTATE;
UARTSTATE State;
int command_length;
int command_count;
int motor_count;
int motor_length;
void uart_recv_int_handler() {
#ifdef __USE18F26J50
    if (DataRdy1USART()) {
        uc_ptr->buffer[uc_ptr->buflen] = Read1USART();
#else
#ifdef __USE18F46J50
    if (DataRdy1USART()) {
        uc_ptr->buffer[uc_ptr->buflen] = Read1USART();
#else
    if (DataRdyUSART()) {
        uc_ptr->buffer[uc_ptr->buflen] = ReadUSART();
#endif
#endif
        uc_ptr->buflen++;
        parseUART();
        
        // check if a message should be sent
    }
#ifdef __USE18F26J50
    if (USART1_Status.OVERRUN_ERROR == 1) {
#else
#ifdef __USE18F46J50
    if (USART1_Status.OVERRUN_ERROR == 1) {
#else
    if (USART_Status.OVERRUN_ERROR == 1) {
#endif
#endif
        // we've overrun the USART and must reset
        // send an error message for this
        RCSTAbits.CREN = 0;
        RCSTAbits.CREN = 1;
        ToMainLow_sendmsg(0, MSGT_OVERRUN, (void *) 0);
    }
}

void init_uart_recv(uart_comm *uc) {
    uc_ptr = uc;
    uc_ptr->buflen = 0;
    State = MSGCOUNT;
}
void parseUART()
{
    switch(State)
    {
        case(MSGCOUNT):
        {
            State = MSGID;
            command_count = 0;
            command_length = 0;
            motor_count = 0;
            break;
        }
        case(MSGID):
        {
            command_length = uc_ptr->buffer[uc_ptr->buflen-1] & 0x0f;
            if(command_length != 0)
            {
            State = COMMAND;
            }
            else
            {
            State = CHECKSUM;
            }
            break;
        }
        case(COMMAND):
        {
            if(command_count+1 < command_length)
            {command_count++;}
            else
            {State = CHECKSUM;}
            break;
        }
        case(CHECKSUM):
        {
            ToMainLow_sendmsg(sizeof(unsigned char)*(3 + command_length),MSGT_PARSE,uc_ptr->buffer);
            uc_ptr->buflen = 0;
            State = MSGCOUNT;
            break;
        }
    }
}