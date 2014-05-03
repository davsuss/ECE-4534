#include "myTimers.h"

/* **************************************************************** */
// WARNING: Do not print in this file -- the stack is not large enough for this task
/* **************************************************************** */

/* *********************************************************** */
// Functions for the Move Task related timer
//
// how often the timer that sends messages to the LCD task should run
// Set the task up to run every 50 ms
#define MoveWRITE_RATE_BASE	( ( portTickType ) 50 / portTICK_RATE_MS)

// Callback function that is called by the Timer
//   Sends a message to the queue that is read by the Move Task
void moveTimerCallback(xTimerHandle pxTimer)
{
	if (pxTimer == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	} else {	
		// When setting up this timer, I put the pointer to the 
		//   MoveT structure as the "ID" so that I could access
		//   that structure here -- which I need to do to get the 
		//   address of the message queue to send to 
		MoveTaskStruct *ptr = (MoveTaskStruct *) pvTimerGetTimerID(pxTimer);
		// Make this non-blocking *but* be aware that if the queue is full, this routine
		// will not care, so if you care, you need to check something
		if (SendTimerMsgToMoveTask(ptr, MoveWRITE_RATE_BASE, 0) == errQUEUE_FULL) {
			// Here is where you would do something if you wanted to handle the queue being full
			VT_HANDLE_FATAL_ERROR(0);
		}
	}
}

void startTimer(MoveTaskStruct *moveT) {
	if (sizeof(long) != sizeof(MoveTaskStruct *)) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	xTimerHandle MoveTimerHandle = xTimerCreate((const signed char *)"Move Timer", 
												MoveWRITE_RATE_BASE,
												pdTRUE,
												(void *) moveT,
												moveTimerCallback);
	if (MoveTimerHandle == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	} else {
		if (xTimerStart(MoveTimerHandle,0) != pdPASS) {
			VT_HANDLE_FATAL_ERROR(0);
		}
	}
}

/* *********************************************************** */
// Functions for the Move Task related timer
//
// how often the timer that sends messages to the LCD task should run
// Set the task up to run every 50 ms
#define MapWRITE_RATE_BASE	( ( portTickType ) 50 / portTICK_RATE_MS)

// Callback function that is called by the Timer
//   Sends a message to the queue that is read by the Move Task
void mapTimerCallback(xTimerHandle pxTimer)
{
	if (pxTimer == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	} else {	
		// When setting up this timer, I put the pointer to the 
		//   MoveT structure as the "ID" so that I could access
		//   that structure here -- which I need to do to get the 
		//   address of the message queue to send to 
		mapTStruct *ptr = (mapTStruct *) pvTimerGetTimerID(pxTimer);
		// Make this non-blocking *but* be aware that if the queue is full, this routine
		// will not care, so if you care, you need to check something
		if (SendTimerMsgToMapTask(ptr, MoveWRITE_RATE_BASE, 0) == errQUEUE_FULL) {
			// Here is where you would do something if you wanted to handle the queue being full
			VT_HANDLE_FATAL_ERROR(0);
		}
	}
}

void startMapTimer(mapTStruct *mapT) {
	if (sizeof(long) != sizeof(mapTStruct *)) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	xTimerHandle MapTimerHandle = xTimerCreate((const signed char *)"Map Timer", 
												MapWRITE_RATE_BASE,
												pdTRUE,
												(void *) mapT,
												mapTimerCallback);
	if (MapTimerHandle == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	} else {
		if (xTimerStart(MapTimerHandle,0) != pdPASS) {
			VT_HANDLE_FATAL_ERROR(0);
		}
	}
}