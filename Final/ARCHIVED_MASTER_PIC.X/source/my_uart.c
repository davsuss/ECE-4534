#include "maindefs.h"
#ifndef __XC8
#include <usart.h>
#else
#include <plib/usart.h>
#endif
#include "my_uart.h"
#include "Parser_thread.h"
static uart_comm *uc_ptr;



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
        buffer_temp[buf_len] = ReadUSART();
#endif
#endif
        uc_ptr->buflen++;
        buf_len++;
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
    buf_len = 0;
    command_length = 0;
    command_count = 0;
    State = GET_MSGID;
}

void parseUART()
{
    unsigned char x = uc_ptr->buflen;
   // uart_write(1, &x);
    switch(State)
    {
        case(GET_MSGID):
        {
            command_length = buffer_temp[buf_len -1] & 0xF;
            command_length = command_length;
            if(command_length != 0)
            {
                State = GET_COMMAND;
            }
            else
            {
                State = CHECKSUM;
            }
            break;
        }
        case(GET_COMMAND):
        {
            if(command_count+1 < command_length)
            {
                command_count++;
            }
            else
            {
                State = CHECKSUM;
            }
            break;
        }
        case(CHECKSUM):
        {
            ToMainLow_sendmsg(buf_len ,MSGT_PARSE, &buffer_temp[0]);
            uc_ptr->buflen = 0;
            buf_len = 0;
            State = GET_MSGID;
            command_count = 0;
            command_length = 0;
            break;
        }
    }
}


void uart_write(unsigned char length, unsigned char *msg){

    unsigned char i = 0;
    for(i = 0; i < length; i++){

    #ifdef __USE18F26J50
        while(Busy1USART());
        Write1USART(msg[i]);
    #else
    #ifdef __USE18F46J50
        while(Busy1USART());
        Write1USART(msg[i]);
    #else
        while(BusyUSART());
        WriteUSART(msg[i]);
    #endif
    #endif
    }
}