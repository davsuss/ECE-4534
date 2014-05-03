#ifndef MAPTASK_H
#define MAPTASK_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "projdefs.h"
#include "semphr.h"

/* include files. */
#include "vtUtilities.h"
#include "vtI2C.h"
#include "moveTask.h"
#include "webservertask.h" 
#include "roverKinematics.h"

/* Message Types */
#define WheelDistance_ToMAP 0
#define Command_ToMAP 1
#define StFin_ToMAP 2
#define timer_ToMAP 3
#define prob_ToMAP 4

/* Rover State */
#define undefined 0
#define run_1 1
#define run_2 2
#define done 3

// initial length of array that stores the map
#define initialMapArrayLength 150

typedef struct __LastCmdStruct {
	// series of commands with corresponding turn degrees and distances(one byte for feet and one inches)
	uint8_t mp_command; 
	uint8_t mp_turn;
	uint8_t mp_turnDeg;
} LastCmdStruct;

// Structure used to pass parameters to the task
// Do not touch...
typedef struct __mapTStruct {
	MoveTaskStruct *moveT;
	webServerTaskStruct *webServerData;
	xQueueHandle inQ; // input to task
	//variables for storing map of the path
	rvPoseStruct map_array[initialMapArrayLength]; // array to store map
	uint32_t numOfNodesInMap; // number of actual vectors define in the map
	uint32_t segmentInMap;
	LastCmdStruct lastCmdSent; // the last command that was sent
	rvPoseStruct curPose; //Rover's pose - struct imported from roverKinematics.h 
	uint32_t rovState; // rov state (run1, run2, or undefined)
	uint32_t timeElapsedFromStart; // ticks (50ms) since start was sent
	int distTravelled;
} mapTStruct;

#define mapQLen 15

void setTimerFlag(uint8_t value);

// Public API
//
// The job of this task is to read from the message queue that is output by the I2C thread 
// analyze the sensor values and make a decision and send to the move Task
// Start the task
// Args:
//   uxPriority -- the priority you want this task to be run at
void startMapTask(mapTStruct *thisTask, unsigned portBASE_TYPE uxPriority, 
					MoveTaskStruct *moveT);

//
// Send a timer message to the Move task - to poll the 
// Args:
//   moveT -- a pointer to the internals of the MoveTask
//   ticksElapsed -- number of ticks since the last message (this will be sent in the message)
//   ticksToBlock -- how long the routine should wait if the queue is full
// Return:
//   Result of the call to xQueueSend()
portBASE_TYPE SendTimerMsgToMapTask(mapTStruct *mapT, 
									portTickType ticksElapsed,
									portTickType ticksToBlock);

//
// Send wheel distances received from the rover as a result of a sensor request
// Args:
//   Result of the call to xQueueSend()
portBASE_TYPE sendWheelDistancesOnly(mapTStruct *mapT,  
								uint8_t leftDistanceFeet, // distance the left wheel travelled in feet 
								uint8_t leftDistanceInches, // distance the left wheel travelled left over inches 
								uint8_t rightDistanceFeet, // distance the right wheel travelled in feet 
								uint8_t rightDistanceInches, // distance the right wheel travelled left over inches 
								portTickType ticksToBlock);

//
// This method should be called when need to update the command
// Situation: When the rover is waiting for a command and  sensor request is sent.
//			  As a result, the rover replies with sensors and distances. The distances and the new command should be
//			  provided to the map task. The map task will basically remember each command to determine whether
//			  the upcoming distances are for a turn or a straight.
// Args:
//   Result of the call to xQueueSend()
portBASE_TYPE sendNewCommand(mapTStruct *mapT,
								uint8_t command, // pointer to buffer with commands
								uint8_t turnType, // buffer with corresponding turn types
								uint8_t turnDegree, // buffer with corresponding turn degrees
								portTickType ticksToBlock);

// Send a msgDropped message to the move task
// Args:
//   moveT -- a pointer to the internals of the MoveTask
// Return:
//   Result of the call to xQueueSend()
portBASE_TYPE SendStFinNotification(mapTStruct *mapT, 
								portTickType ticksToBlock);

// Send a msgDropped message to the move task
// Args:
//   moveT -- a pointer to the internals of the MoveTask
// Return:
//   Result of the call to xQueueSend()
portBASE_TYPE encounteredProblemWithMap(mapTStruct *mapT, 
								portTickType ticksToBlock);

int clearDistance(mapTStruct *mapT, 
								portTickType ticksToBlock);
void printDebug();
#endif