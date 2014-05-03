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
#include "communicationConstants.h"
#include "vtUtilities.h"
//#include "lcdTask.h"

//-----------------------------------------------------------------------------------------------------
//moveTask messge types (message types sent to moveTask)
#define timer_ToMv 0 // timer message
//#define sensorRequestToRover_ToMv 2 // send sensor request msgID0 (never used to cut down time)
//#define pollArmPic_ToMv 3 // send poll to ARM-pic for buffered info msgID1 (never used)
#define command_ToMv 4 // command to be sent to Rover  msgID2 
#define stopSending_ToMv 5
#define startSending_ToMv 6
#define msgDropped_ToMv 8 // notification that a message was dropped
//-----------------------------------------------------------------------------------------------------
//moveTask_States
#define st1 0
#define st2	1
#define st3	2
//-----------------------------------------------------------------------------------------------------
//command send states
#define started 0 
#define stopped 1 // start button not pressed (doesn't send commands)

// Maximum length of a message that can be received by this task
#define MoveInputMaxLen   (sizeof(portTickType))

// length of maessage at a max
//#define	maxFormattedMsgLength 5

// Length of the queue for this task
#define moveQLen 10
// masximum number of commands(4 bits to determine) to be sent in one message (each command is one byte)
#define maxCommandBytes 15
#define maxNumCommands 3
// number of bytes to represent the check sum
#define CheckSumLen 1
// number of bytes for the msgID and the length
#define msgID_Len 1 
// maximum that could be received
#define maxPossibleReceive 22 
//number of dropped messages till resend
#define dropsTilResend 10
//number of timer interrupts till ARM-PIC poll
#define ticksTillArmPicPoll 7
//number of timer interrupts till Rover sensor poll
#define ticksTillRoverSensorPoll 10

//-----------------------------------------------------------------------------------------------------

typedef struct __LastMsgStruct {
	uint8_t lastMsg_buffer[maxCommandBytes + CheckSumLen + msgID_Len];
	uint8_t lastMsg_length;
	uint8_t lastMsg_i2cType;
	uint8_t lastMsg_recvLength;
} LastMsgStruct;

// Structure used to hold things for the Task the inQ and the outQ
typedef struct __MoveTaskStruct {
	vtI2CStruct *i2cDev; // output to i2c task
	uint32_t numOfTimerInts; // number of timer interrupts
	uint32_t task_state; // state of the task
	uint32_t msgsDropped; // number of dropped messages
	LastMsgStruct lastMsgSent; // store the last message to resend if needed
	xQueueHandle inQ; // input to task
	uint32_t cmdSendState; // 0 = start needs to be started, 1 = start has been sent (keep track of time) 	
} MoveTaskStruct;

// Public API
//
// Start the task
// Args:
//   thisTask: Data structure used by the task to hold out and in Queue
//   uxPriority -- the priority you want this task to be run at
//   i2c: pointer to the data structure for an i2c task
void startMoveTask(MoveTaskStruct *thisTask, 
				unsigned portBASE_TYPE uxPriority, 
				vtI2CStruct *i2c);
//
// Send a timer message to the Move task - to poll the 
// Args:
//   moveT -- a pointer to the internals of the MoveTask
//   ticksElapsed -- number of ticks since the last message (this will be sent in the message)
//   ticksToBlock -- how long the routine should wait if the queue is full
// Return:
//   Result of the call to xQueueSend()
portBASE_TYPE SendTimerMsgToMoveTask(MoveTaskStruct *moveT, 
									portTickType ticksElapsed,
									portTickType ticksToBlock);
//
// Send a command message to the move task
// Args:
//   moveT -- a pointer to the internals of the MoveTask
//   command -- command to send to the
// 	 degreeOfTurn -- how many degrees to turn 
//	 distance -- one byte is for feet and the next for inches
//   ticksToBlock -- how long the routine should wait if the queue is full
// Return:
//   Result of the call to xQueueSend()
portBASE_TYPE SendMoveCommand(MoveTaskStruct *moveT, 
								uint8_t *commandsToSend, // first block in communicationConstants.h
								uint8_t *turnIDs, 		// second block in communicationConstants.h
								uint8_t *degreeOfTurns,
								uint8_t *distances,
								uint8_t numOfCmds, 
								portTickType ticksToBlock);

//
// Send a command message to the move task
// Args:
//   moveT -- a pointer to the internals of the MoveTask
//   command -- command to send to the
// 	 degreeOfTurn -- how many degrees to turn 
//	 distance -- one byte is for feet and the next for inches
//   ticksToBlock -- how long the routine should wait if the queue is full
// Return:
//   Result of the call to xQueueSend()
portBASE_TYPE SendStopToRover(MoveTaskStruct *moveT,  
								portTickType ticksToBlock);

//
// Send a command message to the move task
// Args:
//   moveT -- a pointer to the internals of the MoveTask
//   command -- command to send to the
// 	 degreeOfTurn -- how many degrees to turn 
//	 distance -- one byte is for feet and the next for inches
//   ticksToBlock -- how long the routine should wait if the queue is full
// Return:
//   Result of the call to xQueueSend()
portBASE_TYPE SendStopToMvTask(MoveTaskStruct *moveT,  
								portTickType ticksToBlock);

//
// Send a command message to the move task
// Args:
//   moveT -- a pointer to the internals of the MoveTask
//   command -- command to send to the
// 	 degreeOfTurn -- how many degrees to turn 
//	 distance -- one byte is for feet and the next for inches
//   ticksToBlock -- how long the routine should wait if the queue is full
// Return:
//   Result of the call to xQueueSend()
portBASE_TYPE SendStartToMvTask(MoveTaskStruct *moveT,  
								portTickType ticksToBlock);

// Send a msgDropped message to the move task
// Args:
//   moveT -- a pointer to the internals of the MoveTask
// Return:
//   Result of the call to xQueueSend()
portBASE_TYPE SendDroppedNotification(MoveTaskStruct *moveT, 
								portTickType ticksToBlock);
#endif