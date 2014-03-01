#include "mapTask.h"

/* *********************************************** */
// definitions and data structures that are private to this file

// I have set this to a large stack size because of (a) using printf() and (b) the depth of function calls
//   for some of the i2c operations	-- almost certainly too large, see LCDTask.c for details on how to check the size
#define INSPECT_STACK 1
#define baseStack 2
#if PRINTF_VERSION == 1
#define mapSTACK_SIZE		((baseStack+5)*configMINIMAL_STACK_SIZE)
#else
#define mapSTACK_SIZE		(baseStack*configMINIMAL_STACK_SIZE)
#endif

// actual data structure that is sent in a message
typedef struct __mapTMsg {
	uint8_t msgType;	
} mapTMsg;
// end of defs
/* *********************************************** */

#include "debugPins.h"
bool pinState_mapTtest;
uint32_t pinMask_mapTtest = 0x1; // pin 0

/* The i2cTemp task. */
static portTASK_FUNCTION_PROTO( mapTask, pvParameters );

/*-----------------------------------------------------------*/
// Public API
void startMapTask(mapTStruct *mapT, unsigned portBASE_TYPE uxPriority, MoveTaskStruct *moveT, unsigned int unit_distance)
{ 
	// Create the queue that will be used to talk to this task
	if ((mapT->inQ = xQueueCreate(mapQLen,sizeof(mapTMsg))) == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}

	/* Start the task */
	portBASE_TYPE retval;
	mapT->moveT = moveT;
	mapT->unit_distance = unit_distance;

	if ((retval = 
				xTaskCreate( mapTask, 
					( signed char * ) "Map", mapSTACK_SIZE, (void *) mapT, 
					uxPriority, ( xTaskHandle * ) NULL )) != pdPASS) {
		VT_HANDLE_FATAL_ERROR(retval);
	}

	pinState_mapTtest = initPin();
	GPIO_SetDir(0, pinMask_mapTtest, 1);
}

portBASE_TYPE testMap(mapTStruct *mapT, portTickType ticksToBlock)
{
	if (mapT == NULL) { // make sure proper task pointer was sent
		VT_HANDLE_FATAL_ERROR(0);
	}	

	mapTMsg mapBuf;
	mapBuf.msgType = 0xFF; // signify this is a test msg	

	return(xQueueSend(mapT->inQ,(void *) (&mapBuf),ticksToBlock));
}

// End of Public API
/*-----------------------------------------------------------*/ 
// This is the actual task that is run
static portTASK_FUNCTION( mapTask, pvParameters )
{
	// Get the parameters
	mapTStruct *mapTPtr = (mapTStruct *) pvParameters;

	// Buffer for receiving messages
	mapTMsg msgBuf;

	// Like all good tasks, this should never exit
	for(;;)
	{
		// Wait for a timer or a command message
		if (xQueueReceive(mapTPtr->inQ,(void *) &msgBuf,portMAX_DELAY) != pdTRUE) {
			VT_HANDLE_FATAL_ERROR(0);
		}

		switch( msgBuf.msgType )
		{
			case 0xFF:
			{
				// toggle debug pin
				(pinState_mapTtest == True) ? (GPIO_ClearValue(0, pinMask_mapTtest)) : (GPIO_SetValue(0, pinMask_mapTtest));
				pinState_mapTtest = updatePin(pinState_mapTtest);
				break;
			}
			default:
			{
				VT_HANDLE_FATAL_ERROR(0);
			}
		}
	}
}