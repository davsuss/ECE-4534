// This is where the "user" interrupts handlers should go
// The *must* be declared in "user_interrupts.h"

#include "maindefs.h"
#ifndef __XC8
#include <timers.h>
#else
#include <plib/timers.h>
#endif
#include "user_interrupts.h"
#include "messages.h"
#include "my_i2c.h"
// A function called by the interrupt handler
// This one does the action I wanted for this program on a timer0 interrupt
int x = 0;
void timer0_int_handler() {
    unsigned char val = 0x32;
    int length, msgtype;
    // toggle an LED
    // reset the timer
    WriteTimer0(0);
    if(x == 0)
    {
        x= 1;
    //i2c_configure_master(0x4D);
    //i2c_master_recv(0x6);
    }
    //ToMainHigh_sendmsg(sizeof (val), MSGT_UART_SEND, (void *) &val);
}

// A function called by the interrupt handler
// This one does the action I wanted for this program on a timer1 interrupt

void timer1_int_handler() {
    unsigned int result;

    result = ReadTimer1();

    //ToMainLow_sendmsg(0, MSGT_TIMER1, (void *) 0);

    // reset the timer
    WriteTimer1(0);
}