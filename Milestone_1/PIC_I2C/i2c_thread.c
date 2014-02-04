#include "maindefs.h"
#include <stdio.h>
#include "i2c_thread.h"
#include <plib/i2c.h>
#include "debug.h"

void init_i2c_lthread(i2c_thread_struct *tptr) {
    tptr->data = 0;
}

int i2c_lthread(i2c_thread_struct *tptr, int msgtype, int length, unsigned char *msgbuffer) {
}
