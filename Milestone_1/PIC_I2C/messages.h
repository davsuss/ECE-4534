#ifndef __messages
#define __messages

// The maximum length (in bytes) of a message
#define MSGLEN 10

// The maximum number of messages in a single queue
#define MSGQUEUELEN 4

typedef struct __msg {
	unsigned char	full;
	unsigned char	length;
	unsigned char   msgtype;
	unsigned char   data[MSGLEN];
} msg;

typedef struct __msg_queue {
	msg	queue[MSGQUEUELEN];
	unsigned char	cur_write_ind;
	unsigned char	cur_read_ind;
} msg_queue;

// list of queues in the system
// queues are instantiated in main.c
extern msg_queue timer0_queue;
extern msg_queue timer1_queue;
extern msg_queue i2c_queue;
// Error Codes
// Too many messages in the queue
#define MSGQUEUE_FULL -1
// Message sent okay
#define MSGSEND_OKAY 1
// The length of the message is either too large or negative
#define MSGBAD_LEN -2
// The message buffer is too small to receive the message in the queue
#define MSGBUFFER_TOOSMALL -3
// The message queue is empty
#define MSGQUEUE_EMPTY -4
// This call must be made from a low-priority interrupt handler

void     init_queue(msg_queue *qptr);

// exit codes:
//   MSGBAD_LEN      message length too long
//   MSGQUEUE_FULL   queue full
//   MSGSEND_OKAY    success
signed   char send_msg(msg_queue *qptr, unsigned char length, unsigned char msgtype, void *data);

// exit codes:
//   MSGBUFFER_TOOSMALL  message longer than buffer provided
//   MSGQUEUE_EMPTY      no messsage in queue
//   integer             length of message received
signed   char recv_msg(msg_queue *qptr, unsigned char maxlength, unsigned char *msgtype, void *data);

// exit codes:
//   nonzero             message is available
unsigned char check_msg(msg_queue *qptr);

#endif
