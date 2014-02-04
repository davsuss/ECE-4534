#ifndef __user_interrupts
#define __user_interrupts

// interrupts defined by the "user" and that are called from
// interrupts.c -- the "user" needs to insert these into
// interrupts.c because it, of course, doesn't know which
// interrupt handlers you would like to call

void timer0_int_handler(void);

void timer1_int_handler(void);

void I2CR_int_handler(void);
#endif
