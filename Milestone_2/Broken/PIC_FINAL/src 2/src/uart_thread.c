#include "maindefs.h"
#include <stdio.h>
#include "uart_thread.h"
#include "my_i2c.h"
#include "my_uart.h"

// This is a "logical" thread that processes messages from the UART
// It is not a "real" thread because there is only the single main thread
// of execution on the PIC because we are not using an RTOS.

int uart_lthread(uart_thread_struct *uptr, unsigned char msgtype, signed char length, unsigned char *msgbuffer) {
    if (msgtype == MSGT_OVERRUN) {
    }
    else if (msgtype == MSGT_UART_DATA) {
        // print the message (this assumes that the message
        // 		was a printable string)
        //msgbuffer[length] = '\0'; // null-terminate the array as a string
        // Now we would do something with it

        // Find if it matches the checksum
        unsigned char commandlength = (msgbuffer[1] & 0xF);
        unsigned char checksum = msgbuffer[length - 1];

        unsigned char a = 'a';
        uart_write(1, &a);
        uart_write(1, &length);

        unsigned char checker = 0;
        for (unsigned char i = 0; i < length; i++)
        {
            checker = checker + msgbuffer[i];
        }

        if ((checker & 0xFF) == checksum)
        {
            unsigned char messageID = (msgbuffer[1] & 0xF0) >> 4;
            if (messageID == 0x0){
                // Sensor data request
            }
            else if (messageID == 0x1){
                // This should never happen
            }
            else if (messageID == 0x2){
                // Motor command
                for (unsigned char j = 0; j < commandlength; j++){
                    unsigned char command = msgbuffer[1 + j];
                    unsigned char cmdID = (command & 0xF0);
                    unsigned char turn = (command & 0xF);

                    unsigned int start = 0x7;

                    if (cmdID == 0x0){
                        // start
                        //i2c_master_send(1, &start);
                    }
                    else if (cmdID == 0x1){
                        // stop
                    }
                    else if (cmdID == 0x2) {
                        //straight
                    }
                    else if (cmdID == 0x3) {
                        // turn right
                    }
                    else if (cmdID == 0x4) {
                        // turn left
                    }
                }
            }
        }
    }

}
