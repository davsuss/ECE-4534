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
        getsUSART(uc_ptr->buffer, uc_ptr->buflen);
#endif
#endif
//Handle_i2c_data_save
        uc_ptr->buflen = MAXUARTBUF;
        flipDBG(2);
        // check if a message should be sent
        //if (uc_ptr->buflen == MAXUARTBUF) {
            ToMainLow_sendmsg(uc_ptr->buflen, MSGT_UART_DATA, (void *) uc_ptr->buffer);
            //uc_ptr->buflen = 0;
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
  

    //putsUSART requires buffer to terminate with a null
    //character.
    msg[length] = '\0';

    //putsUSART will write the message buffer until it
    //sees a null character.
    //while(BusyUSART());
    //WriteUSART(msg[0]);

    unsigned char i = 0;
    for(i = 0; i < length; i++){
        while(BusyUSART());
        WriteUSART(msg[i]);
    }
}

void retrieve_command(){

    switch(STATE){
        case GET_MSG_ID:
        {
            MSG_LEN = 0;
            STATE = GET_MSG_ID;
            break;
        };

        case GET_MSG_COUNT:
        {
    
            STATE = GET_SAMPLE;
            break;
        };

        case GET_SAMPLE:
        {
            if(MSG_LEN < uc_ptr->buffer[uc_ptr->buflen - 2 - MSG_LEN])
                MSG_LEN++;
            else {
                MSG_LEN = 0;
                STATE = GET_MOTOR_ROTATION;
            }
            break;
        };

        case GET_MOTOR_ROTATION:
        {
            if(MSG_LEN < MOTOR_ROTATIONS)
                MSG_LEN++;
            else {
                STATE = CHECK_SUM;
            }
            break;
        };

        case CHECK_SUM:
        {
            break;
        };
    };
}
