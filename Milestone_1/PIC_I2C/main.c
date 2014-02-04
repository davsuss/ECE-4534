#include "maindefs.h"
#include "i2c_thread.h"
#include <stdio.h>
#include <plib/timers.h>
#include <plib/i2c.h>
#include "interrupts.h"
#include "messages.h"
#include "timer1_thread.h"
#include "timer0_thread.h"
#include "debug.h"
#include "I2C.h"

#ifdef __USE18F45J10
// CONFIG1L
#pragma config WDTEN = OFF      // Watchdog Timer Enable bit (WDT disabled (control is placed on SWDTEN bit))
#pragma config STVREN = OFF     // Stack Overflow/Underflow Reset Enable bit (Reset on stack overflow/underflow disabled)
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode enabled)

// CONFIG1H
#pragma config CP0 = OFF        // Code Protection bit (Program memory is not code-protected)

// CONFIG2L
#pragma config FOSC = HSPLL     // Oscillator Selection bits (HS oscillator, PLL enabled and under software control)
#pragma config FOSC2 = ON       // Default/Reset System Clock Select bit (Clock selected by FOSC as system clock is enabled when OSCCON<1:0> = 00)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor enabled)
#pragma config IESO = ON        // Two-Speed Start-up (Internal/External Oscillator Switchover) Control bit (Two-Speed Start-up enabled)

// CONFIG2H
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config CCP2MX = DEFAULT // CCP2 MUX bit (CCP2 is multiplexed with RC1)
#else
#error "Unsupported microcontroller - include CONFIG in main.c"
#endif

// does all configuration except starting the interrupt system
void init_hardware() {
    OSCCON = 0x82;         // see datasheeet
    OSCTUNEbits.PLLEN = 0; // Makes the clock exceed the PIC's rated speed if the PLL is on

    // set direction for PORTB to output
    TRISB = 0x0;
    LATB = 0x0;

    // initialize Timers
    OpenTimer0(TIMER_INT_ON   &
               T0_16BIT       &
               T0_SOURCE_INT  &
               T0_PS_1_2    );

    OpenTimer1(TIMER_INT_ON   &
               T1_PS_1_2      &
               T1_16BIT_RW    &
               T1_SOURCE_INT  &
               T1_OSC1EN_OFF  &
               T1_SYNC_EXT_OFF);

    // Decide on the priority of the enabled peripheral interrupts
    // 0 is low, 1 is high


    //I2C

    /////



}

void main(void) {
    signed   char length;
    unsigned char msgtype;
    unsigned char msgbuffer[MSGLEN + 1];

    init_hardware();

    timer1_thread_struct t1thread_data; // info for timer1_lthread
    timer0_thread_struct t0thread_data; // info for timer0_lthread
    i2c_thread_struct i2cthread_data;
    // initialize message queues before enabling any interrupts
    init_queue(&timer0_queue);
    init_queue(&timer1_queue);
    init_queue(&i2c_queue);
    InitI2C();
    // initialize thread local data
    init_timer1_lthread(&t1thread_data);
    init_timer0_lthread(&t0thread_data);
    init_i2c_lthread(&i2cthread_data);
    enable_interrupts();
    
    // loop forever
    // - If there are no pending messages in the queues, sleep
    // - Otherwise, read and dispatch messages to "threads"

    while (1) {
        // signal queue check
        
        //I2c
        length = recv_msg(&i2c_queue,MSGLEN,&msgtype,(void*)msgbuffer);
        if(length < 0)
        {}
        else
        {switch(msgtype)
        {
            case MSGT_I2C:
                i2c_lthread(&i2cthread_data,msgtype,length,msgbuffer);
                break;
            default:
                flipDBG(DBG4);

        }

        }
        //////


        // check timer0 queue
        length = recv_msg(&timer0_queue, MSGLEN, &msgtype, (void *) msgbuffer);
        if (length < 0) {
            // no message - check error code
            if (length != MSGQUEUE_EMPTY) {
                // This case be handled by your code.
            }
        } else {
            switch (msgtype) {
                case MSGT_TIMER0: 
                    timer0_lthread(&t0thread_data, msgtype, length, msgbuffer);
                    break;
                default:
                    // unrecognized message flips 4
                    flipDBG(DBG4);
                    break;
            }
        }

        // signal queue check
        setDBG(DBG5);
        resetDBG(DBG5);

        // check timer1 queue
        length = recv_msg(&timer1_queue, MSGLEN, &msgtype, (void *) msgbuffer);
        if (length < 0) {
            // no message - check error code
            if (length != MSGQUEUE_EMPTY) {
                // This case be handled by your code.
            }
        } else {
            switch (msgtype) {
                case MSGT_TIMER1:
                    timer1_lthread(&t1thread_data, msgtype, length, msgbuffer);
                    break;
                default:
                    // unrecognized message
                    flipDBG(DBG5);
                    break;
            }
        }

    }

}