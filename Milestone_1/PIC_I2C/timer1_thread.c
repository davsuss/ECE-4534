#include "maindefs.h"
#include <stdio.h>
#include "messages.h"
#include "timer1_thread.h"
#include "debug.h"

void init_timer1_lthread(timer1_thread_struct *tptr) {
    tptr->data = 0;
}

int timer1_lthread(timer1_thread_struct *tptr, int msgtype, int length, unsigned char *msgbuffer) {
    
}
