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
#include "Motor_thread.h"
// A function called by the interrupt handler
// This one does the action I wanted for this program on a timer0 interrupt


void timer0_int_handler() {


    // toggle an LED
#ifdef __USE18F2680
    LATBbits.LATB0 = !LATBbits.LATB0;
#endif
    // reset the timer
    WriteTimer0(0);
    NumOfOverflowL++;
    DeltaOverflowL++;
    


    if(NumOfOverflowL >= OverflowsL && ActiveL == 1)
    {
     ToMainHigh_sendmsg(0, MSGT_MOTOR_STOP, (void *) 0);
     ActiveL = 0;
    }

    // try to receive a message and, if we get one, echo it back
     //result = ReadTimer1();
    

}

// A function called by the interrupt handler
// This one does the action I wanted for this program on a timer1 interrupt

void timer1_int_handler() {
    unsigned int result = 0x04;

    // read the timer and then send an empty message to main()
#ifdef __USE18F2680
    LATBbits.LATB1 = !LATBbits.LATB1;
#endif
    NumOfOverflowR++;
    DeltaOverflowR++;
    if(NumOfOverflowR >= OverflowsR && ActiveR == 1)
    {
     ActiveR = 0;
     ToMainHigh_sendmsg(0, MSGT_MOTOR_STOP, (void *) 0);
    }


    // reset the timer
    WriteTimer1(0);
}