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
#include "debug.h"

// A function called by the interrupt handler
// This one does the action I wanted for this program on a timer0 interrupt
int x = 0;
int i = 0;
int length = 0;
void timer0_int_handler() {
    unsigned int val;
    int length, msgtype;

    // toggle an LED
#ifdef __USE18F2680
    LATBbits.LATB0 = !LATBbits.LATB0;
#endif
    // reset the timer
    WriteTimer0(0);
    //unsigned char *name = "ar";
    //flipDBG(0);
    //uart_write(2,name);
   
    // try to receive a message and, if we get one, echo it back
   // length = FromMainHigh_recvmsg(sizeof(val), (unsigned char *)&msgtype, (void *) &val);
   // if (length == sizeof (val)) {
    ConvertADC();
    //ADCON0bits.GO = 1;
    //ToMainLow_sendmsg(sizeof (val), MSGT_ADC_START, (void *) &val);
   // }
}

// A function called by the interrupt handler
// This one does the action I wanted for this program on a timer1 interrupt

void timer1_int_handler() {
    unsigned int result;

    // read the timer and then send an empty message to main()
#ifdef __USE18F2680
    LATBbits.LATB1 = !LATBbits.LATB1;
#endif

    result = ReadTimer1();
    ToMainLow_sendmsg(0, MSGT_TIMER1, (void *) 0);

    // reset the timer
    WriteTimer1(0);
}
void adc_int_handler()
{
    unsigned int result;
    result = ReadADC();
    result = result /4;
    //Handle_i2c_data_save(1,&result);
    //unsigned char buf = 'a';
    //ToMainLow_sendmsg(5, MSGT_UART_DATA, (void *) buf);
    //uart_write(1,&buf);
}