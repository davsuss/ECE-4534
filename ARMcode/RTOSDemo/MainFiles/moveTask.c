/* include files. */
#include "moveTask.h"

/* *********************************************** */
// definitions and data structures that are private to this file
 
// actual data structure that is sent in a message
typedef struct __moveTMsg {
	uint8_t msgType;
	uint8_t command_buf[maxNumCommands]; // series of commands with corresponding turn degrees and distances(one byte for feet and one inches)
	uint8_t turn_buf[maxNumCommands];
	uint8_t turnDeg_buf[maxNumCommands];
	uint8_t distance_buf[maxNumCommands*2];
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

bool pinState_cmd;
uint32_t pinMask_cmd = 0x20000; // 17

// end of defs
/* *********************************************** */

/* The i2cTemp task. */
static portTASK_FUNCTION_PROTO( moveRoverTask, pvParameters );

/*-----------------------------------------------------------*/

void initLastMsg(LastMsgStruct *msgStruct)
{
	msgStruct->lastMsg_length = 0;
	msgStruct->lastMsg_i2cType = 0;
	msgStruct->lastMsg_recvLength = 0;
}
// Public API
void startMoveTask(MoveTaskStruct *thisTask,unsigned portBASE_TYPE uxPriority, vtI2CStruct *i2c)
{
	// Create the queue that will be used to talk to this task
	if ((thisTask->inQ = xQueueCreate(moveQLen,sizeof(moveTMsg))) == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	/* Start the task */
	portBASE_TYPE retval;
	thisTask->i2cDev = i2c; // set the i2c task that this task will use to send commands
	thisTask->numOfTimerInts = 0;
	thisTask->msgsDropped = 0;	
	thisTask->task_state = st1;
	thisTask->cmdSendState = stopped;
	
	LastMsgStruct lastMsg;
	initLastMsg(&lastMsg);
	thisTask->lastMsgSent = lastMsg; 										  

	pinState_s1 = initPin();
	pinState_s2 = initPin();
	pinState_cmd = initPin();
	GPIO_SetDir(0, pinMask_s1, 1);
	GPIO_SetDir(0, pinMask_s2, 1);
	GPIO_SetDir(0, pinMask_cmd, 1);

	if ((retval = xTaskCreate( moveRoverTask, ( signed char * ) "moveT", moveSTACK_SIZE, (void *) thisTask, uxPriority, ( xTaskHandle * ) NULL )) != pdPASS) {
		VT_HANDLE_FATAL_ERROR(retval);
	}
}

portBASE_TYPE SendTimerMsgToMoveTask(MoveTaskStruct *moveT, portTickType ticksElapsed, portTickType ticksToBlock)
{
	//printf("Pass\n");
	if (moveT == NULL) { // make sure proper task pointer was sent
		VT_HANDLE_FATAL_ERROR(0);
	}	

	moveTMsg moveBuf;
	moveBuf.msgType = timer_ToMv; // signify this is a timer msg			

	return(xQueueSend(moveT->inQ,(void *) (&moveBuf),ticksToBlock));
}

portBASE_TYPE SendStopToRover(MoveTaskStruct *moveT,  
								portTickType ticksToBlock)
{
	uint8_t cmd [] = {stop_cmd_ToRover};
	uint8_t t [] = {0};
	uint8_t deg [] = {0};
	uint8_t dist [] = {0,0};
	SendMoveCommand(moveT, cmd, t, deg, dist, 1, 0);
}

// [cmd1, cmd2], [type1, type2], [turn1, turn2], [dist_feet1, dist_inch1, dist_feet2, dist_inches2], numCmds = 2
portBASE_TYPE SendMoveCommand(MoveTaskStruct *moveT, 
								uint8_t *commandsToSend, // first block in communicationConstants.h
								uint8_t *turnIDs, 		// second block in communicationConstants.h
								uint8_t *degreeOfTurns,
								uint8_t *distances,
								uint8_t numOfCmds, 
								portTickType ticksToBlock)
{
	if( moveT->cmdSendState == started )
	{
		if (moveT == NULL) {
			VT_HANDLE_FATAL_ERROR(0);
		}
		moveTMsg moveBuf;
		moveBuf.msgType = command_ToMv; // signify this is a command msg
		int i = 0;	
		for( i = 0; i < numOfCmds; i++ )
		{
			moveBuf.command_buf[i] = commandsToSend[i];
			moveBuf.turn_buf[i] = turnIDs[i];
			moveBuf.turnDeg_buf[i] = degreeOfTurns[i];
			moveBuf.distance_buf[2*i] = distances[2*i];
			moveBuf.distance_buf[2*i + 1] = distances[2*i + 1];
		}
		moveBuf.numOfCmds = numOfCmds;	 
		return(xQueueSend(moveT->inQ,(void *) (&moveBuf),ticksToBlock));
	}
	else
	{
		return pdTRUE;
	}
}

portBASE_TYPE SendStopToMvTask(MoveTaskStruct *moveT,  
								portTickType ticksToBlock)
{
	if (moveT == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	moveTMsg moveBuf;
	moveBuf.msgType = stopSending_ToMv;
	return(xQueueSend(moveT->inQ,(void *) (&moveBuf),ticksToBlock));
}

portBASE_TYPE SendStartToMvTask(MoveTaskStruct *moveT,  
								portTickType ticksToBlock)
{
	if (moveT == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
	moveTMsg moveBuf;
	moveBuf.msgType = startSending_ToMv;	
	return(xQueueSend(moveT->inQ,(void *) (&moveBuf),ticksToBlock));
}

portBASE_TYPE SendDroppedNotification(MoveTaskStruct *moveT, 
								portTickType ticksToBlock)
{
	if (moveT == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}

	moveTMsg moveBuf;
	moveBuf.msgType = msgDropped_ToMv; // signify this is a msgDropped notification

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

void sendPackagedMsg(vtI2CStruct *i2cdevPtr, uint8_t i2cMsgType, uint8_t length, const uint8_t *msgBuf, uint8_t recvLength)
{
	// send message
	// i2cdevPtr - the i2c task
	// cmdSend - msg type that will be written as a means of debugging
	// 0x4F - slave address
	// msgBuf has the message to be sent
	if (vtI2CEnQ(i2cdevPtr, i2cMsgType, (uint8_t)0x4F, length, msgBuf, recvLength) != pdTRUE) {
		VT_HANDLE_FATAL_ERROR(0);
	}
}

// private functions used to send  sensor data request meant to be sent to Rover
void sensorRequestFromRover(vtI2CStruct *i2cdevPtr, MoveTaskStruct *moveT)
{
	uint8_t id = (uint8_t)sensorReq_msgID_ToRover;	
	// wrap message in buffer
	// middle 1 is for the msgID << 4 | cmdLength
	// final 1 byte for the checkSum
	// will never send commands when requesting for sensor data

	(moveT->lastMsgSent).lastMsg_buffer[0] = id << 4; // id is MSnibble while the length is the least significant | 0 commands
	(moveT->lastMsgSent).lastMsg_buffer[1] = calcCheckSum((moveT->lastMsgSent).lastMsg_buffer, msgID_Len);
	(moveT->lastMsgSent).lastMsg_length = CheckSumLen + msgID_Len;
	(moveT->lastMsgSent).lastMsg_i2cType = sensor1;
	(moveT->lastMsgSent).lastMsg_recvLength = 0;
	// no commands

	sendPackagedMsg(i2cdevPtr, 
				(moveT->lastMsgSent).lastMsg_i2cType, 
				(moveT->lastMsgSent).lastMsg_length, 
				(const uint8_t *)(moveT->lastMsgSent).lastMsg_buffer, 
				(moveT->lastMsgSent).lastMsg_recvLength);
}

// private function to send sensor data request meant to retrieve received data from Arm-PIC
void sensorRequestFromARMPic(vtI2CStruct *i2cdevPtr, MoveTaskStruct *moveT)
{
	uint8_t id = (uint8_t)sesorReplyPoll_msgID_ToARMPic;		
	// wrap message in buffer
	// will never send commands when requesting for sensor data
	uint8_t msgBuf[msgID_Len + CheckSumLen];

	msgBuf[0] = id << 4; // id is MSnibble while the length is the least significant
	msgBuf[1] = calcCheckSum(msgBuf, msgID_Len);
	// no commands

	sendPackagedMsg(i2cdevPtr,
					sensor2,
					sizeof(msgBuf),
					(const uint8_t *)msgBuf,
					maxPossibleReceive);
}

// private function to send sensor data request meant to retrieve received data from Arm-PIC
void sendStopCommand(vtI2CStruct *i2cdevPtr, MoveTaskStruct *moveT)
{
	uint8_t id = (uint8_t)sendStop_msgID_ToRover;	

	moveT->task_state = st2;
	moveT->numOfTimerInts = 0;

	// toggle debug pin
	(pinState_cmd == True) ? (GPIO_ClearValue(0, pinMask_cmd)) : (GPIO_SetValue(0, pinMask_cmd));
	pinState_cmd = updatePin(pinState_cmd);

	(moveT->lastMsgSent).lastMsg_buffer[0] = id << 4; // id is MSnibble while the length is the least significant | 0 commands
	(moveT->lastMsgSent).lastMsg_buffer[1] = calcCheckSum((moveT->lastMsgSent).lastMsg_buffer, msgID_Len);
	(moveT->lastMsgSent).lastMsg_length = CheckSumLen + msgID_Len;
	(moveT->lastMsgSent).lastMsg_i2cType = cmdSend;
	(moveT->lastMsgSent).lastMsg_recvLength = 0;
	// no commands

	sendPackagedMsg(i2cdevPtr, 
				(moveT->lastMsgSent).lastMsg_i2cType, 
				(moveT->lastMsgSent).lastMsg_length, 
				(const uint8_t *)(moveT->lastMsgSent).lastMsg_buffer, 
				(moveT->lastMsgSent).lastMsg_recvLength);
}

// private function to send sensor data request meant to retrieve received data from Arm-PIC
void sendStartCommand(vtI2CStruct *i2cdevPtr, MoveTaskStruct *moveT)
{
	uint8_t id = (uint8_t)sendStart_msgID_ToRover;	

	moveT->task_state = st2;
	moveT->numOfTimerInts = 0;

	// toggle debug pin
	(pinState_cmd == True) ? (GPIO_ClearValue(0, pinMask_cmd)) : (GPIO_SetValue(0, pinMask_cmd));
	pinState_cmd = updatePin(pinState_cmd);

	(moveT->lastMsgSent).lastMsg_buffer[0] = id << 4; // id is MSnibble while the length is the least significant | 0 commands
	(moveT->lastMsgSent).lastMsg_buffer[1] = calcCheckSum((moveT->lastMsgSent).lastMsg_buffer, msgID_Len);
	(moveT->lastMsgSent).lastMsg_length = CheckSumLen + msgID_Len;
	(moveT->lastMsgSent).lastMsg_i2cType = cmdSend;
	(moveT->lastMsgSent).lastMsg_recvLength = 0;
	// no commands

	sendPackagedMsg(i2cdevPtr, 
				(moveT->lastMsgSent).lastMsg_i2cType, 
				(moveT->lastMsgSent).lastMsg_length, 
				(const uint8_t *)(moveT->lastMsgSent).lastMsg_buffer, 
				(moveT->lastMsgSent).lastMsg_recvLength);
}

// private function to send commands - need to pass in buffer of commands
// [cmd1, cmd2], [type1, type2], [turn1, turn2], [dist_feet1, dist_inch1, dist_feet2, dist_inches2], numCmds = 2
void sendCommand(vtI2CStruct *i2cdevPtr,
                 MoveTaskStruct *moveT,
                 uint8_t *cmd_ptr,
                 uint8_t *turnID_ptr,
                 uint8_t *turnD_ptr,
                 uint8_t *dist_ptr,
                 unsigned int numCmds)
{
	int i;
	uint8_t id = (uint8_t)sendCommand_msgID_ToRover;
	uint8_t length = 0;
    
	moveT->lastMsgSent.lastMsg_buffer[0] = id << 4 | (numCmds*4 & 0xF);// each command has 4 bytes
	length++;
    
	for( i = 0; i < numCmds; i++ )
	{
//		printf("cmd send_cmd %d\n", cmd[0]);
//		printf("turn send_cmd %d\n", turn[0]);
		(moveT->lastMsgSent).lastMsg_buffer[length+0] = (cmd_ptr[i] & 0xF << 4) | (turnID_ptr[i] & 0xF); //command ID is MSN and turn type is LSN
		(moveT->lastMsgSent).lastMsg_buffer[length+1] = turnD_ptr[i]; //turn degrees 0 - 180
		(moveT->lastMsgSent).lastMsg_buffer[length+2] = dist_ptr[2*i]; //feet
		(moveT->lastMsgSent).lastMsg_buffer[length+3] = dist_ptr[2*i+1]; //inches
		length+=4;// 4bytes per command
	}
    
	(moveT->lastMsgSent).lastMsg_buffer[length] = calcCheckSum((moveT->lastMsgSent).lastMsg_buffer, length);
	length++;
	moveT->task_state = st2;
	moveT->numOfTimerInts = 0;

	(moveT->lastMsgSent).lastMsg_length = (uint8_t)length;
	(moveT->lastMsgSent).lastMsg_i2cType = cmdSend;
	(moveT->lastMsgSent).lastMsg_recvLength = 0;
	// no commands

	sendPackagedMsg(i2cdevPtr, 
				(moveT->lastMsgSent).lastMsg_i2cType, 
				(moveT->lastMsgSent).lastMsg_length, 
				(const uint8_t *)(moveT->lastMsgSent).lastMsg_buffer, 
				(moveT->lastMsgSent).lastMsg_recvLength);
	
	// toggle debug pin
	(pinState_cmd == True) ? (GPIO_ClearValue(0, pinMask_cmd)) : (GPIO_SetValue(0, pinMask_cmd));
	pinState_cmd = updatePin(pinState_cmd);
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

		//printf("state = %d\n", task->rovState);

		// Now, based on the type of the message and the state, we decide on the new state and action to take
		switch(msgBuf.msgType) {
		case timer_ToMv: 
		{
			(task->numOfTimerInts)++; //increment ticks counter	
			break;
		}
		case command_ToMv: 
		{
			sendCommand(i2cdevPtr, task, 
						msgBuf.command_buf,
						msgBuf.turn_buf, 
						msgBuf.turnDeg_buf, 
						msgBuf.distance_buf, 
						msgBuf.numOfCmds);			
			break;
		}
		case stopSending_ToMv:
		{
			task->cmdSendState = stopped;
			break;
		}
		case startSending_ToMv:
		{
			task->cmdSendState = started;
			break;
		}
		case msgDropped_ToMv:
		{
			(task->msgsDropped)++;
			if( task->msgsDropped % 5 == 0 )
			{
				//printf("len in = %d\n",(task->lastMsgSent).lastMsg_length); 
				//resend msg
				sendPackagedMsg(i2cdevPtr, 
								(task->lastMsgSent).lastMsg_i2cType, 
								(task->lastMsgSent).lastMsg_length,
								(const uint8_t *)(task->lastMsgSent).lastMsg_buffer,
								(task->lastMsgSent).lastMsg_recvLength);	
			}
			//printf("len out = %d\n",(task->lastMsgSent).lastMsg_length); 
			//printf("buf out = %d\n",(task->lastMsgSent).lastMsg_buffer[0]);
			task->task_state = st2;
			task->numOfTimerInts = 0;
			break;
		}
		}

		//state machine to switch states
		switch(task->task_state){
		case st1:
		{
			if( task->numOfTimerInts >= ticksTillRoverSensorPoll )//&& task->pollSt == enabled )
			{
				(pinState_s1 == True) ? (GPIO_ClearValue(0, pinMask_s1)) : (GPIO_SetValue(0, pinMask_s1));
				pinState_s1 = updatePin(pinState_s1);

				// send I2C
				sensorRequestFromRover(i2cdevPtr, task);
				
				//change states
				task->task_state = st2;
				task->numOfTimerInts = 0;			
			}
			break;
		}
		case st2:
		{
			if( task->numOfTimerInts >= ticksTillArmPicPoll )//&& task->pollSt == enabled )
			{
				// drive debug pins high				
				(pinState_s2 == True) ? (GPIO_ClearValue(0, pinMask_s2)) : (GPIO_SetValue(0, pinMask_s2));
				pinState_s2 = updatePin(pinState_s2);

				// send test I2C msg
				sensorRequestFromARMPic(i2cdevPtr, task);

				// state transition
				task->task_state = st1;
				task->numOfTimerInts = 0;			
			}
			break;
		}
		case st3:
		{
			if( task->numOfTimerInts >= 7 )
			{
				// drive debug pins high				
				(pinState_s2 == True) ? (GPIO_ClearValue(0, pinMask_s2)) : (GPIO_SetValue(0, pinMask_s2));
				pinState_s2 = updatePin(pinState_s2);

				// send test I2C msg
				sensorRequestFromARMPic(i2cdevPtr, task);

				// state transition
				task->task_state = st1;
				task->numOfTimerInts = 0;			
			}
			break;
		}
		}
		 
	}
}
