// This is where the "user" interrupts handlers should go
// The *must* be declared in "user_interrupts.h"

#include "maindefs.h"
#include <plib/timers.h>
#include "user_interrupts.h"
#include "messages.h"
#include "debug.h"

// A function called by the interrupt handler
// This one does the action I wanted for this program on a timer0 interrupt

void timer0_int_handler() {
    // toggle an output
    setDBG(DBG1);

    // send a message down the queue
#if 1
    send_msg(&timer0_queue, 0, MSGT_TIMER0, (void *) 0);
#else
    // shows how to to overflow detection for debugging
    if (send_msg(&timer0_queue, 0, MSGT_TIMER0, (void *) 0) == MSGQUEUE_FULL) {
       setDBG(DBG4);
       setDBG(DBG5);
       flipDBG(DBG1);
       flipDBG(DBG1);
       flipDBG(DBG1);
       flipDBG(DBG1);
       flipDBG(DBG1);
       flipDBG(DBG1);
    }
#endif

    // reset the timer - writing 0xF000 makes the timer overflow after 1K ticks
    WriteTimer0(0xF000);
    resetDBG(DBG1);
}

void timer1_int_handler() {
    // read the timer and then send an empty message to main()
    setDBG(DBG2);

    // send a message down the queue
    send_msg(&timer1_queue, 0, MSGT_TIMER1, (void *) 0);

    // reset the timer - writing 0xF000 makes the timer overflow after 1K ticks
    WriteTimer1(0xF000);
    resetDBG(DBG2);
}
