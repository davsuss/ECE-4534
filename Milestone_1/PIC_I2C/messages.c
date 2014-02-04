#include "maindefs.h"
#include "messages.h"
#include <string.h>
#include "debug.h"
msg_queue i2c_queue;
msg_queue timer0_queue;
msg_queue timer1_queue;

void init_queue(msg_queue *qptr) {
    unsigned char i;

    qptr->cur_write_ind = 0;
    qptr->cur_read_ind = 0;
    for (i = 0; i < MSGQUEUELEN; i++) {
        qptr->queue[i].full = 0;
    }
}

signed char send_msg(msg_queue *qptr, unsigned char length, unsigned char msgtype, void *data) {
    unsigned char slot;
    msg *qmsg;
    unsigned char tlength = length;

    if (length > MSGLEN) {
        return (MSGBAD_LEN);
    }

    slot = qptr->cur_write_ind;
    qmsg = &(qptr->queue[slot]);

    // if the slot isn't empty, then we should return
    if (qmsg->full != 0) {
        return (MSGQUEUE_FULL);
    }

    // now fill in the message
    qmsg->length = length;
    qmsg->msgtype = msgtype;

    memcpy(qmsg->data, data, tlength);
    qptr->cur_write_ind = (qptr->cur_write_ind + 1) % MSGQUEUELEN;

    // This *must* be done after the message is completely inserted
    qmsg->full = 1;
    return (MSGSEND_OKAY);
}

signed char recv_msg(msg_queue *qptr, unsigned char maxlength, unsigned char *msgtype, void *data) {
    unsigned char slot;
    msg *qmsg;
    size_t tlength;

    // check to see if anything is available
    slot = qptr->cur_read_ind;
    qmsg = &(qptr->queue[slot]);
    if (qmsg->full == 1) {

        // not enough room in the buffer provided
        if (qmsg->length > maxlength) {
            return (MSGBUFFER_TOOSMALL);
        }

        // now actually copy the message
        tlength = qmsg->length;
        memcpy(data, qmsg->data, tlength);
        qptr->cur_read_ind = (qptr->cur_read_ind + 1) % MSGQUEUELEN;
        (*msgtype) = qmsg->msgtype;

        // this must be done after the message is completely extracted
        qmsg->full = 0;
        return (tlength);

    } else {
        return (MSGQUEUE_EMPTY);
    }
}

// check if message available
unsigned char check_msg(msg_queue *qptr) {
    return (qptr->queue[qptr->cur_read_ind].full);
}
