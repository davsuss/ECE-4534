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

// Structure used to pass parameters to the task
// Do not touch...
typedef struct __mapTStruct {
	unsigned int unit_distance;	
	MoveTaskStruct *moveT;
	xQueueHandle inQ; // input to task
} mapTStruct;

#define mapQLen 10

// Public API
//
// The job of this task is to read from the message queue that is output by the I2C thread 
// analyze the sensor values and make a decision and send to the move Task
// Start the task
// Args:
//   uxPriority -- the priority you want this task to be run at
void startMapTask(mapTStruct *thisTask, unsigned portBASE_TYPE uxPriority, 
					MoveTaskStruct *moveT, unsigned int unit_distance);
//
// Send a command message to the map task to test
// Args:
//   Result of the call to xQueueSend()
portBASE_TYPE testMap(mapTStruct *mapT, portTickType ticksToBlock);
#endif