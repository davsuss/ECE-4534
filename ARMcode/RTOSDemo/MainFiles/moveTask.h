#ifndef MOVETASK_H
#define MOVETASK_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "projdefs.h"
#include "semphr.h"

// interface task includes
#include "vtI2C.h"
#include "I2CTaskMsgTypes.h"
#include "vtUtilities.h"
//#include "lcdTask.h"

typedef enum{timer = 0, command, msgDropped} moveMsgType; // type of message sent to the task
typedef enum{start = 0, stop, straight, right, left} commandType; // type of command to be sent to rover
typedef enum{noTurn = 0, forty_five, ninety} turnType; // byte representation of the turn commands
typedef enum{waitToSendSensor1 = 0, waitToSendSensor2} states; // state machine for requesting sensor info
typedef enum{sensorReq1 = 0, sensorReq2, sendCmd} msgID; // message ID sent to the rover

// Structure used to hold things for the Task the inQ and the outQ
typedef struct __MoveTaskStruct {
	vtI2CStruct *i2cDev; // output to i2c task
	unsigned int unit_distance;
	unsigned int numOfTimerInts;
	states st;
	uint8_t msgsDropped;
	uint8_t msgCount;
	xQueueHandle inQ; // input to task
} MoveTaskStruct;


// Maximum length of a message that can be received by this task
#define MoveInputMaxLen   (sizeof(portTickType))

// length of maessage at a max
//#define	maxFormattedMsgLength 5

// Length of the queue for this task
#define moveQLen 10

// masximum number of commands(4 bits to determine) to be sent in one message (each command is one byte)
#define maxCommands 15
// number of bytes to represent the msgCount
#define MsgCountLen 1 
// number of bytes to represent the check sum
#define CheckSumLen 1
// number of bytes for the msgID and the length
#define msgID_Len 1 
#define maxPossibleReceive 22 // maximum that could be received 

// Public API
//
// Start the task
// Args:
//   thisTask: Data structure used by the task to hold out and in Queue
//   uxPriority -- the priority you want this task to be run at
//   i2c: pointer to the data structure for an i2c task
//	 unit_distance -- unit distance the rover is expected to move for each command (number of rotations)
void startMoveTask(MoveTaskStruct *thisTask, unsigned portBASE_TYPE uxPriority, vtI2CStruct *i2c, unsigned int unit_distance);
//
// Send a timer message to the Move task - to poll the 
// Args:
//   moveT -- a pointer to the internals of the MoveTask
//   ticksElapsed -- number of ticks since the last message (this will be sent in the message)
//   ticksToBlock -- how long the routine should wait if the queue is full
// Return:
//   Result of the call to xQueueSend()
portBASE_TYPE SendTimerMsgToMoveTask(MoveTaskStruct *moveT, portTickType ticksElapsed, portTickType ticksToBlock);
//
// Send a command message to the move task
// Args:
//   moveT -- a pointer to the internals of the MoveTask
//   command -- command to send to the
// 	 degreeOfTurn -- how many degrees to turn 
//   ticksToBlock -- how long the routine should wait if the queue is full
// Return:
//   Result of the call to xQueueSend()
portBASE_TYPE SendMoveCommand(MoveTaskStruct *moveT, 
								commandType *commandSent, 
								turnType *degreeOfTurn,
								uint8_t numOfCmds, 
								portTickType ticksToBlock);

// Send a msgDropped message to the move task
// Args:
//   moveT -- a pointer to the internals of the MoveTask
// Return:
//   Result of the call to xQueueSend()
portBASE_TYPE SendDroppedNotification(MoveTaskStruct *moveT, 
								portTickType ticksToBlock);

#endif