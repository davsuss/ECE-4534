#include "maindefs.h"
#ifndef __XC8
#include <usart.h>
#else
#include <plib/usart.h>
#endif
#include "my_uart.h"
#include "debug.h"

static uart_comm *uc_ptr;

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
//Handle_i2c_data_save
        uc_ptr->buflen++;
        retrieve_command();
        // check if a message should be sent
        //if (uc_ptr->buflen == 5) {
        //    ToMainLow_sendmsg(uc_ptr->buflen, MSGT_UART_DATA, (void *) uc_ptr->buffer);
        //    uc_ptr->buflen = 0;
        //}
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
}

void uart_write(unsigned char length, unsigned char *msg){

    flipDBG(4);

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

void retrieve_command(){

    flipDBG(2);
    switch(STATE){

        case GET_MSG_ID_AND_LEN:
        {
            MSG_LEN = uc_ptr->buffer[uc_ptr->buflen - 1] & 0xF;

            if(MSG_LEN == 0)
                STATE = GET_DISTANCE_FEET_LEFT;
            else
                STATE = GET_SAMPLE;
            
            break;
        };

        case GET_SAMPLE:
        {
            if(MSG_REC < MSG_LEN - 1)
                MSG_REC++;
            else {
                MSG_REC = 0;
                MSG_LEN = 0;
                STATE = GET_DISTANCE_FEET_LEFT;
            }

            break;
        };

        case GET_DISTANCE_FEET_LEFT:
        {
            STATE = GET_DISTANCE_INCHES_LEFT;
            break;
        };

        case GET_DISTANCE_INCHES_LEFT:
        {
            STATE = GET_DISTANCE_FEET_RIGHT;
            break;
        };

        case GET_DISTANCE_FEET_RIGHT:
        {
            STATE = GET_DISTANCE_INCHES_RIGHT;
            break;
        };

        case GET_DISTANCE_INCHES_RIGHT:
        {
            STATE = GET_CHECK_SUM;
            break;
        };

        case GET_CHECK_SUM:
        {
            MSG_LEN = 0;
            MSG_REC = 0;
            STATE = GET_MSG_ID_AND_LEN;
            ToMainLow_sendmsg(uc_ptr->buflen, MSGT_UART_DATA, (void *) uc_ptr->buffer);
            uc_ptr->buflen = 0;
            break;
        };
    };
}
