#include "maindefs.h"
#include <stdio.h>
#include "uart_thread.h"
#include "plib/usart.h"
// This is a "logical" thread that processes messages from the UART
// It is not a "real" thread because there is only the single main thread
// of execution on the PIC because we are not using an RTOS.

int uart_lthread(uart_thread_struct *uptr, int msgtype, int length, unsigned char *msgbuffer) {
    if (msgtype == MSGT_OVERRUN) {
    }
    else if (msgtype == MSGT_UART_DATA) {
        // print the message (this assumes that the message
        // 		was a printable string)
        msgbuffer[length] = '\0'; // null-terminate the array as a string
        // Now we would do something with it
        
    }
    else if(msgtype == MSGT_UART_SEND)
    {
        //WriteUSART(64);
        int x = 0;
        length /= sizeof(unsigned char);
        for(x = 0; x < length; x++ )
        {
            while(BusyUSART());
            WriteUSART(msgbuffer[x]);
        }
    }
}
