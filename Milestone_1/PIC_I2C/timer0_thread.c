#include "maindefs.h"
#include <stdio.h>
#include "timer0_thread.h"
#include "debug.h"

void init_timer0_lthread(timer0_thread_struct *tptr) {
    tptr->data = 0;
}

int timer0_lthread(timer0_thread_struct *tptr, int msgtype, int length, unsigned char *msgbuffer) {
    // timer0 messages will flip
    
}