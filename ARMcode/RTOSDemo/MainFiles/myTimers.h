#ifndef _MY_TIMERS_H
#define _MY_TIMERS_H

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "projdefs.h"
#include "timers.h"

/* include files. */
#include "vtUtilities.h"
#include "moveTask.h"

void startTimerForTemperature(MoveTaskStruct *moveT);
#endif