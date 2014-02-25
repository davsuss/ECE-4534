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
        //getsUSART(uc_ptr->buffer, uc_ptr->buflen);
        uc_ptr->buffer[uc_ptr->buflen] = ReadUSART();
#endif
#endif
//Handle_i2c_data_save
        uc_ptr->buflen++;
        flipDBG(0);
        //retrieve_command();
        // check if a message should be sent
        //if (uc_ptr->buflen == 4) {
        //    ToMainLow_sendmsg(uc_ptr->buflen, MSGT_UART_DATA, (void *) uc_ptr->buffer);
        //    uc_ptr->buflen = 0;
        //}

        if(uc_ptr->buflen == 2)
            COMMAND_LEN = uc_ptr->buffer[uc_ptr->buflen - 1] & 0xF;

        if(COMMAND_REC < COMMAND_LEN)
            COMMAND_REC++;
        else {
            COMMAND_LEN = 0;
            COMMAND_REC = 0;
            ToMainLow_sendmsg(uc_ptr->buflen, MSGT_UART_DATA, (void *) uc_ptr->buffer);
            uc_ptr->buflen = 0;
        }

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
    STATE = GET_MSG_COUNT;
    COMMAND_LEN = 0;
    COMMAND_REC = 0;

}

void uart_write(unsigned char length, unsigned char *msg){
    
    unsigned char i = 0;
    for(i = 0; i < length; i++){
        while(BusyUSART());
        WriteUSART(msg[i]);
    }
}

//enum {GET_MSG_COUNT, GET_MSG_ID_AND_LEN, GET_COMMANDS};
void retrieve_command(){

    switch(STATE){

        case GET_MSG_COUNT:
        {
            COMMAND_LEN = 0;
            COMMAND_REC = 0;

            //ToMainLow_sendmsg(uc_ptr->buflen, MSGT_UART_DATA,
            //                  (void *) uc_ptr->buffer);

            STATE = GET_MSG_ID_AND_LEN;

            break;
        };

        case GET_MSG_ID_AND_LEN:
        {
            COMMAND_LEN = uc_ptr->buffer[uc_ptr->buflen - 1] & 0xF;

            //ToMainLow_sendmsg(1, MSGT_UART_DATA,
             //                 &STATE);

            //ToMainLow_sendmsg(uc_ptr->buflen, MSGT_UART_DATA,
            //                  (void *) uc_ptr->buffer);

            STATE = GET_COMMANDS;
        };

        case GET_COMMANDS:
        {
            if(COMMAND_REC < COMMAND_LEN){
                COMMAND_REC++;
                //ToMainLow_sendmsg(uc_ptr->buflen, MSGT_UART_DATA,
                //              (void *) uc_ptr->buffer);
            }
            else {
                 ToMainLow_sendmsg(uc_ptr->buflen, MSGT_UART_DATA,
                              (void *) uc_ptr->buffer);
                uc_ptr->buflen = 0;

                //ToMainLow_sendmsg(1, MSGT_UART_DATA,
                //              &STATE);

                STATE = GET_MSG_COUNT;
            }
            
            break;
        };
    };
}
