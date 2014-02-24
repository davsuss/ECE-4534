#include "maindefs.h"
#include <stdio.h>
#include "timer0_thread.h"
#include "my_i2c.h"
// This is a "logical" thread that processes messages from TIMER0
// It is not a "real" thread because there is only the single main thread
// of execution on the PIC because we are not using an RTOS.
int i = 0;
int j = 100;
int swit = 0;
int length = 0;
int timer0_lthread(timer0_thread_struct *tptr, int msgtype, int length, unsigned char *msgbuffer) {
    unsigned int *msgval;
    unsigned int val = 0;

    val = i++;

    if(i > 255)
    {
        i = 0;
    }

    msgval = (unsigned int *) msgbuffer;   
    ToMainLow_sendmsg(sizeof(i),MSGT_I2C_DATA_SAVE,(void*)&val);


    // Here is where we would do something with the message

}

