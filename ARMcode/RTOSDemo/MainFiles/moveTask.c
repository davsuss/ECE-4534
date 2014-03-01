/* include files. */
#include "moveTask.h"

/* *********************************************** */
// definitions and data structures that are private to this file
 
// actual data structure that is sent in a message
typedef struct __moveTMsg {
	moveMsgType msgType;
	commandType *command; // series of commands with corresponding turnTypes
	turnType *turn;
	uint8_t numOfCmds;	
} moveTMsg;

// I have set this to a large stack size because of (a) using printf() and (b) the depth of function calls
//   for some of the i2c operations	-- almost certainly too large, see LCDTask.c for details on how to check the size
#define baseStack 3
#if PRINTF_VERSION == 1
#define moveSTACK_SIZE		((baseStack+5)*configMINIMAL_STACK_SIZE)
#else
#define moveSTACK_SIZE		(baseStack*configMINIMAL_STACK_SIZE)
#endif

//Debug PIcs
#include "debugPins.h"
bool pinState_s1;
uint32_t pinMask_s1 = 0x8000; // 15

bool pinState_s2;
uint32_t pinMask_s2 = 0x10000; // 16

bool pinState_dropped;
uint32_t pinMask_dropped = 0x20000; // 17

// end of defs
/* *********************************************** */

/* The i2cTemp task. */
static portTASK_FUNCTION_PROTO( moveRoverTask, pvParameters );

/*-----------------------------------------------------------*/
// Public API
void startMoveTask(MoveTaskStruct *thisTask,unsigned portBASE_TYPE uxPriority, vtI2CStruct *i2c, unsigned int unit_distance)
{
	// Create the queue that will be used to talk to this task
	if ((thisTask->inQ = xQueueCreate(moveQLen,sizeof(moveTMsg))) == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	/* Start the task */
	portBASE_TYPE retval;
	thisTask->unit_distance = unit_distance; // set the unit distance
	thisTask->i2cDev = i2c; // set the i2c task that this task will use to send commands
	thisTask->st = waitToSendSensor1;
	thisTask->numOfTimerInts = 0;
	thisTask->msgCount = 0;
	thisTask->msgsDropped = 0;											  

	pinState_s1 = initPin();
	pinState_s2 = initPin();
	pinState_dropped = initPin();
	GPIO_SetDir(0, pinMask_s1, 1);
	GPIO_SetDir(0, pinMask_s2, 1);
	GPIO_SetDir(0, pinMask_dropped, 1);

	if ((retval = xTaskCreate( moveRoverTask, ( signed char * ) "moveT", moveSTACK_SIZE, (void *) thisTask, uxPriority, ( xTaskHandle * ) NULL )) != pdPASS) {
		VT_HANDLE_FATAL_ERROR(retval);
	}
}

portBASE_TYPE SendTimerMsgToMoveTask(MoveTaskStruct *moveT, portTickType ticksElapsed, portTickType ticksToBlock)
{
	if (moveT == NULL) { // make sure proper task pointer was sent
		VT_HANDLE_FATAL_ERROR(0);
	}	

	moveTMsg moveBuf;
	moveBuf.msgType = timer; // signify this is a timer msg
	moveBuf.numOfCmds = 0;
	turnType *dumturn = noTurn;
	moveBuf.turn = dumturn;
	commandType *cmd = start;
	moveBuf.command = cmd;		

	return(xQueueSend(moveT->inQ,(void *) (&moveBuf),ticksToBlock));
}

portBASE_TYPE SendMoveCommand(MoveTaskStruct *moveT, 
								commandType *commandSent, 
								turnType *degreeOfTurn,
								uint8_t numOfCmds, 
								portTickType ticksToBlock)
{
	if (moveT == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}

	moveTMsg moveBuf;
	moveBuf.msgType = command; // signify this is a command msg
	moveBuf.command = commandSent; // signify the command
	moveBuf.turn = degreeOfTurn; // how many degrees to turn
	moveBuf.numOfCmds = numOfCmds;

	return(xQueueSend(moveT->inQ,(void *) (&moveBuf),ticksToBlock));
}

portBASE_TYPE SendDroppedNotification(MoveTaskStruct *moveT, 
								portTickType ticksToBlock)
{
	if (moveT == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}

	moveTMsg moveBuf;
	moveBuf.msgType = msgDropped; // signify this is a msgDropped notification

	//dummy variables
	moveBuf.numOfCmds = 0;
	turnType *dumturn = noTurn;
	moveBuf.turn = dumturn;
	commandType *cmd = start;
	moveBuf.command = cmd;

	return(xQueueSend(moveT->inQ,(void *) (&moveBuf),ticksToBlock));
}

// End of Public API
/*-----------------------------------------------------------*/
uint8_t calcCheckSum(uint8_t *buf, uint32_t len)
{
	uint32_t rollingSum = 0;
	uint32_t i;
	for( i = 0; i < len; i++ )
	{
		 rollingSum += buf[i];
	}
	return rollingSum & 0xFF; // return just LSB
}

// private functions used to send  sensor data request meant to be sent to Rover
void sensorRequestFromRover(vtI2CStruct *i2cdevPtr, MoveTaskStruct *moveT)
{
	msgID id = sensorReq1;	
	// wrap message in buffer
	// middle 1 is for the msgID << 4 | cmdLength
	// final 1 byte for the checkSum
	// will never send commands when requesting for sensor data
	uint8_t msgBuf[MsgCountLen + msgID_Len + CheckSumLen];

	// input the msgCount
	msgBuf[0] = moveT->msgCount;
	msgBuf[1] = id << 4; // id is MSnibble while the length is the least significant
	msgBuf[2] = calcCheckSum(msgBuf, MsgCountLen + 1);
	// no commands

	// send message
	// i2cdevPtr - the i2c task
	// sensor1 - msg type that will be written as a means of debugging
	// 0x4F - slave address
	// msgBuf has the message to be sent
	if (vtI2CEnQ(i2cdevPtr, sensor1,0x4F,sizeof(msgBuf),msgBuf,0) != pdTRUE) {
		VT_HANDLE_FATAL_ERROR(0);
	}

	moveT->msgCount = (moveT->msgCount == 255) ? 0 : moveT->msgCount+1; // update msgCount appropriately
}

// private function to send sensor data request meant to retrieve received data from Arm-PIC
void sensorRequestFromARMPic(vtI2CStruct *i2cdevPtr, MoveTaskStruct *moveT)
{
	//NOTE: Need to change the max bytes rcvd if the number of samples incoming changes
	// set with constraints

	msgID id = sensorReq2;	
	// wrap message in buffer
	// will never send commands when requesting for sensor data
	uint8_t msgBuf[MsgCountLen + msgID_Len + CheckSumLen];

	// input the msgCount
	msgBuf[0] = moveT->msgCount;
	msgBuf[1] = id << 4; // id is MSnibble while the length is the least significant
	msgBuf[2] = calcCheckSum(msgBuf, MsgCountLen + msgID_Len);
	// no commands

	// send message
	// i2cdevPtr - the i2c task
	// sensor1 - msg type that will be written as a means of debugging
	// 0x4F - slave address
	// msgBuf has the message to be sent
	if (vtI2CEnQ(i2cdevPtr, sensor2,0x4F,sizeof(msgBuf),msgBuf,maxPossibleReceive) != pdTRUE) {
		VT_HANDLE_FATAL_ERROR(0);
	}

	moveT->msgCount = (moveT->msgCount == 255) ? 0 : moveT->msgCount+1; // update msgCount appropriately
}

// private function to send commands - need to pass in buffers of commands
void sendCommand(vtI2CStruct *i2cdevPtr, MoveTaskStruct *moveT, commandType *cmd, turnType *turn, unsigned int numCmds)
{
	int i;
	msgID id = sendCmd;
	uint8_t length = 0;	
	// wrap message in buffer
	// third 1 is for the msgID << 4 | cmdLength
	uint8_t msgBuf[maxCommands + MsgCountLen + msgID_Len + CheckSumLen];

	// input the msgCount
	msgBuf[0] = moveT->msgCount; length++;
	msgBuf[1] = id << 4 | (numCmds & 0xF); // there will be two commands if its a turn (degree)
	length++;

	for( i = 0; i < numCmds; i++ )
	{
		msgBuf[2] = cmd[i] << 4 | turn[i]; 
		length++;
	}

	msgBuf[length] = calcCheckSum(msgBuf, length);
	length++;

	// send message
	// i2cdevPtr - the i2c task
	// cmdSend - msg type that will be written as a means of debugging
	// 0x4F - slave address
	// msgBuf has the message to be sent
	if (vtI2CEnQ(i2cdevPtr, cmdSend,0x4F,length,msgBuf,0) != pdTRUE) {
		VT_HANDLE_FATAL_ERROR(0);
	}

	moveT->msgCount = (moveT->msgCount == 255) ? 0 : moveT->msgCount+1; // update msgCount appropriately
}

static portTASK_FUNCTION( moveRoverTask, pvParameters )
{
	// Get the parameters
	MoveTaskStruct *task = (MoveTaskStruct *) pvParameters;

	// Get the I2C device pointer
	vtI2CStruct *i2cdevPtr = task->i2cDev;

	// Buffer for receiving messages
	moveTMsg msgBuf;

	// NOTE: Assumes that the I2C device (and thread) have already been initialized
	// Like all good tasks, this should never exit
	for(;;)
	{
		// Wait for a timer or a command message
		if (xQueueReceive(task->inQ,(void *) &msgBuf,portMAX_DELAY) != pdTRUE) {
			VT_HANDLE_FATAL_ERROR(0);
		}

		// Now, based on the type of the message and the state, we decide on the new state and action to take
		switch(msgBuf.msgType) {
		case timer: 
		{
			(task->numOfTimerInts)++; //increment ticks counter			
			break;
		}
		case command: 
		{
			sendCommand(i2cdevPtr, task, msgBuf.command, msgBuf.turn, 1);
			break;
		}
		case msgDropped:
		{
			printf("Dropped: %d\n", task->msgsDropped);
			(task->msgsDropped)++;

			//Notify for every 5 msgsDropped
			if( (task->msgsDropped % 5) == 0 )
			{
				// toggle debug pin
				(pinState_dropped == True) ? (GPIO_ClearValue(0, pinMask_dropped)) : (GPIO_SetValue(0, pinMask_dropped));
				pinState_dropped = updatePin(pinState_dropped);
			}

			break;
		}
		}
		
		//state machine to switch states
		switch(task->st){
		case waitToSendSensor1:
		{
			if( task->numOfTimerInts > 5 )
			{
				// drive debug pins high
				(pinState_s1 == True) ? (GPIO_ClearValue(0, pinMask_s1)) : (GPIO_SetValue(0, pinMask_s1));
				pinState_s1 = updatePin(pinState_s1);

				// send I2C
				sensorRequestFromRover(i2cdevPtr, task);
				
				//change states
				task->st = waitToSendSensor2;
				task->numOfTimerInts = 0;			
			}
			break;
		}
		case waitToSendSensor2:
		{
			if( task->numOfTimerInts > 3 )
			{
				// drive debug pins high				
				(pinState_s2 == True) ? (GPIO_ClearValue(0, pinMask_s2)) : (GPIO_SetValue(0, pinMask_s2));
				pinState_s2 = updatePin(pinState_s2);

				// send test I2C msg
				sensorRequestFromARMPic(i2cdevPtr, task);

				// state transition
				task->st = waitToSendSensor1;
				task->numOfTimerInts = 0;			
			}
			break;
		}
		}
		 
	}
}
