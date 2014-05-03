#ifndef SENSORANALYZER_H
#define SENSORANALYZER_H

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
#include "I2CTaskMsgTypes.h"
#include "moveTask.h"
#include "mapTask.h"

// possible response types

// Structure used to pass parameters to the task
// Do not touch...
typedef struct __SensorAStruct {
	unsigned int unit_distance; // represents the distance the ARM will clear the ROVER
	vtI2CStruct *i2cDev; // to access the outQ of vtI2C
	MoveTaskStruct *moveT; // to access the inQ of moveTask
	mapTStruct *mapT;  // to access the inQ of mapTas
    webServerTaskStruct *webServerData;
} SensorAStruct;

// maximum number of bytes of sensor samples possible
#define maxSensorBytes 7
// maximum bytes expected to be returned by the ARM-PIC upon request
#define maxReceiveBytesForSpecificNumOfSensorSamples 9 // 3 sensor samples
#define numOfSensorToDebounce 3
#define acceptableTolerance 2


enum {GLOBAL_READ, GETTING_IN_3_FEET, AVOID_OBSTACLE_1};

uint8_t current_state = GLOBAL_READ;
uint8_t	prev_state = GLOBAL_READ;

// Public API
//
// The job of this task is to read from the message queue that is output by the I2C thread 
// analyze the sensor values and make a decision and send to the move Task
// Start the task
// Args:
//   conductorData: Data structure used by the task
//   uxPriority -- the priority you want this task to be run at
//   i2c: pointer to the data structure for an i2c task
//	 moveT: move task to write commands to
//	 mapT: send the same commands to map task to analyze and store
void startSensorTask(SensorAStruct *thisTask, unsigned portBASE_TYPE uxPriority, vtI2CStruct *i2c, MoveTaskStruct *moveT, mapTStruct *mapT, unsigned int unit_distance);
#endif