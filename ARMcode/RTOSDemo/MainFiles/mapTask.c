#include "mapTask.h"
#include "webservertask.h"

/* *********************************************** */
// definitions and data structures that are private to this file

// I have set this to a large stack size because of (a) using printf() and (b) the depth of function calls
//   for some of the i2c operations	-- almost certainly too large, see LCDTask.c for details on how to check the size
#define INSPECT_STACK 1
#define baseStack 10
#if PRINTF_VERSION == 1
#define mapSTACK_SIZE		((baseStack+5)*configMINIMAL_STACK_SIZE)
#else
#define mapSTACK_SIZE		(baseStack*configMINIMAL_STACK_SIZE)
#endif

// actual data structure that is sent in a message
typedef struct __mapTMsg {
	uint8_t msgType;
	uint8_t rightFeet;
	uint8_t rightInches;
	uint8_t leftFeet;
	uint8_t leftInches;
	// series of commands with corresponding turn degrees and distances(one byte for feet and one inches)
	uint8_t to_mp_command; 
	uint8_t to_mp_turn;
	uint8_t to_mp_turnDeg;	
} mapTMsg;
// end of defs
/* *********************************************** */

#include "debugPins.h"
bool pinState_mapTtest;
uint32_t pinMask_mapTtest = 0x1; // pin 0

uint8_t timerFlag = 0;

void setTimerFlag(uint8_t value)
{
 	timerFlag = value;
}

/* The i2cTemp task. */
static portTASK_FUNCTION_PROTO( mapTask, pvParameters );

/*-----------------------------------------------------------*/
// Public API
void startMapTask(mapTStruct *mapT, unsigned portBASE_TYPE uxPriority, MoveTaskStruct *moveT)
{ 
	// Create the queue that will be used to talk to this task
	if ((mapT->inQ = xQueueCreate(mapQLen,sizeof(mapTMsg))) == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}

/*
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
		uint32_t map_array[initialMapArrayLength]; // array to store map
		uint32_t numOfNodesInMap; // number of actual vectors define in the map
		LastCmdStruct lastCmdSent; // the last command that was sent
	} mapTStruct;
*/
	/* Start the task */
	portBASE_TYPE retval;
	mapT->moveT = moveT;
	(mapT->lastCmdSent).mp_command = start_cmd_ToRover;
	(mapT->lastCmdSent).mp_turn = straight_turn_ToRover;
	(mapT->lastCmdSent).mp_turnDeg = 0;
	(mapT->curPose).x = 0;
	(mapT->curPose).y = 0;
	(mapT->curPose).orientation = 0.0;
	mapT->distTravelled = 0;
	mapT->numOfNodesInMap = 1;
	mapT->segmentInMap = 1;
	mapT->timeElapsedFromStart = 0;
	mapT->rovState = undefined;	

	if ((retval = 
				xTaskCreate( mapTask, 
					( signed char * ) "Map", mapSTACK_SIZE, (void *) mapT, 
					uxPriority, ( xTaskHandle * ) NULL )) != pdPASS) {
		VT_HANDLE_FATAL_ERROR(retval);
	}

	pinState_mapTtest = initPin();
	GPIO_SetDir(0, pinMask_mapTtest, 1);
}

//
// Send wheel distances received from the rover as a result of a sensor request
// Args:
//   Result of the call to xQueueSend()
portBASE_TYPE sendWheelDistancesOnly(mapTStruct *mapT,  
								uint8_t leftDistanceFeet, // distance the left wheel travelled in feet 
								uint8_t leftDistanceInches, // distance the left wheel travelled left over inches 
								uint8_t rightDistanceFeet, // distance the right wheel travelled in feet 
								uint8_t rightDistanceInches, // distance the right wheel travelled left over inches 
								portTickType ticksToBlock)
{
	if (mapT == NULL) { // make sure proper task pointer was sent
		VT_HANDLE_FATAL_ERROR(0);
	}
	
	/*typedef struct __mapTMsg {
	uint8_t msgType;
	uint8_t rightFeet;
	uint8_t rightInches;
	uint8_t leftFeet;
	uint8_t leftInches;
	// series of commands with corresponding turn degrees and distances(one byte for feet and one inches)
	uint8_t mp_command_buf[maxNumCommands]; 
	uint8_t mp_turn_buf[maxNumCommands];
	uint8_t mp_turnDeg_buf[maxNumCommands];
	uint8_t numOfCmds;	
	} mapTMsg;*/	

	mapTMsg mapBuf;
	mapBuf.msgType = WheelDistance_ToMAP; // signify this is a test msg	
	mapBuf.rightFeet = rightDistanceFeet;
	mapBuf.rightInches = rightDistanceInches;
	mapBuf.leftFeet = leftDistanceFeet;
	mapBuf.leftInches = leftDistanceInches;	
	// dont change the commands stored	

	return(xQueueSend(mapT->inQ,(void *) (&mapBuf),ticksToBlock));
}

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
								portTickType ticksToBlock)
{
	if (mapT == NULL) { // make sure proper task pointer was sent
		VT_HANDLE_FATAL_ERROR(0);
	}	
   	/*typedef struct __mapTMsg {
	uint8_t msgType;
	uint8_t rightFeet;
	uint8_t rightInches;
	uint8_t leftFeet;
	uint8_t leftInches;
	// series of commands with corresponding turn degrees and distances(one byte for feet and one inches)
	uint8_t mp_command_buf[maxNumCommands]; 
	uint8_t mp_turn_buf[maxNumCommands];
	uint8_t mp_turnDeg_buf[maxNumCommands];
	uint8_t numOfCmds;	
	} mapTMsg;*/

	mapTMsg mapBuf;
	mapBuf.msgType = Command_ToMAP;	
	
	mapBuf.to_mp_command = command;
	mapBuf.to_mp_turn = turnType;
	mapBuf.to_mp_turnDeg = turnDegree;	

	return(xQueueSend(mapT->inQ,(void *) (&mapBuf),ticksToBlock));
}

//
// This method should be called when need to update the command
// Situation: When the rover is waiting for a command and  sensor request is sent.
//			  As a result, the rover replies with sensors and distances. The distances and the new command should be
//			  provided to the map task. The map task will basically remember each command to determine whether
//			  the upcoming distances are for a turn or a straight.
// Args:
//   Result of the call to xQueueSend()
portBASE_TYPE SendStFinNotification(mapTStruct *mapT, 
								portTickType ticksToBlock)
{
	if (mapT == NULL) { // make sure proper task pointer was sent
		VT_HANDLE_FATAL_ERROR(0);
	}	
   	/*typedef struct __mapTMsg {
	uint8_t msgType;
	uint8_t rightFeet;
	uint8_t rightInches;
	uint8_t leftFeet;
	uint8_t leftInches;
	// series of commands with corresponding turn degrees and distances(one byte for feet and one inches)
	uint8_t mp_command_buf[maxNumCommands]; 
	uint8_t mp_turn_buf[maxNumCommands];
	uint8_t mp_turnDeg_buf[maxNumCommands];
	uint8_t numOfCmds;	
	} mapTMsg;*/

	mapTMsg mapBuf;
	mapBuf.msgType = StFin_ToMAP;		

	return(xQueueSend(mapT->inQ,(void *) (&mapBuf),ticksToBlock));
}

portBASE_TYPE SendTimerMsgToMapTask(mapTStruct *mapT, 
									portTickType ticksElapsed,
									portTickType ticksToBlock)
{
	//printf("Pass\n");
	if (mapT == NULL) { // make sure proper task pointer was sent
		VT_HANDLE_FATAL_ERROR(0);
	}	

	mapTMsg mapBuf;
	mapBuf.msgType = timer_ToMAP; // signify this is a timer msg			

	return(xQueueSend(mapT->inQ,(void *) (&mapBuf),ticksToBlock));
}

portBASE_TYPE encounteredProblemWithMap(mapTStruct *mapT, 
								portTickType ticksToBlock)
{
	if (mapT == NULL) { // make sure proper task pointer was sent
		VT_HANDLE_FATAL_ERROR(0);
	}	

	mapTMsg mapBuf;
	mapBuf.msgType = prob_ToMAP; // signify this is a timer msg			

	return(xQueueSend(mapT->inQ,(void *) (&mapBuf),ticksToBlock));
}

int clearDistance(mapTStruct *mapT, 
								portTickType ticksToBlock)
{
	if (mapT == NULL) { // make sure proper task pointer was sent
		VT_HANDLE_FATAL_ERROR(0);
	}	
	int distance = 0;
	if(mapT->segmentInMap <= mapT->numOfNodesInMap)
	{ 
		switch( mapT->rovState )
		{
			case run_2:
			{
				int x1 = (mapT->curPose).x;
				int x2 = (mapT->map_array[mapT->segmentInMap]).x;
				int y1 = (mapT->curPose).y;
				int y2 =( mapT->map_array[mapT->segmentInMap-1]).y;
				int d = calcDistance(x1, y1, x2, y2);
				//printf("distCleared = %d\n", d);
				distance = (d > 24) ? 24 : d;
				break;
			}
			default:
			{
				distance = 0;
			}
		}
	}
	return distance;
}

void printDebug(mapTStruct *mapT)
{
	printf("x = %d\n", (mapT->curPose).x);
	printf("y = %d\n", (mapT->curPose).y);
	printf("orientation = %d\n", (int)((double)(mapT->curPose).orientation));
}

// End of Public API
/*-----------------------------------------------------------*/
//node parser 
int getX(rvPoseStruct *map_array, uint32_t numOfNodes)
{
	int elemInArray = numOfNodes - 1;	
	return (map_array[elemInArray]).x;
}
int getY(rvPoseStruct *map_array, uint32_t numOfNodes)
{
	int elemInArray = numOfNodes - 1;	
	return (map_array[elemInArray]).y;
}
float getOrientation(rvPoseStruct *map_array, uint32_t numOfNodes)
{
	int elemInArray = numOfNodes - 1;	
	return (map_array[elemInArray]).orientation;
}
//update nodes
void setX(rvPoseStruct *map_array, uint32_t numOfNodes, int newX)
{
	uint32_t elemInArray = numOfNodes - 1;
//	elemInArray = (elemInArray * singleNodeLen) + x_pos;
//	//printf("oldX = %d\n", map_array[elemInArray]);	
	(map_array[elemInArray]).x = newX;
//	printf("newX");
}
void setY(rvPoseStruct *map_array, uint32_t numOfNodes, int newY)
{
	uint32_t elemInArray = numOfNodes - 1;
	//printf("oldY = %d\n", map_array[elemInArray]);		
	(map_array[elemInArray]).y = newY;
	//printf("newY = %d\n", map_array[1]);
//	printf("newY");
}
void setOrientation(rvPoseStruct *map_array, uint32_t numOfNodes, float newOrientation)
{
	uint32_t elemInArray = numOfNodes - 1;
	//printf("oldO = %d\n", map_array[elemInArray]);	
	(map_array[elemInArray]).orientation = newOrientation;
	//printf("newO = %d\n", map_array[2]);
//	printf("newOrientation");
}

void updateMap(rvPoseStruct *map_array, uint32_t *numOfNodes, mapTStruct *mapT)
{
	switch((mapT->lastCmdSent).mp_turn)
	{
		case straight_turn_ToRover:
		{
			break;
		}
		case left_turn_ToRover:
		{
			if( (mapT->lastCmdSent).mp_turnDeg == 90 )
			{				 
				float oldO = getOrientation(map_array, *numOfNodes);
				(*numOfNodes)++;
				//printf("left Turn 90 num = %d\n", (*numOfNodes));
				setX(map_array, *numOfNodes, (mapT->curPose).x);
				setY(map_array, *numOfNodes, (mapT->curPose).y);
				setOrientation(map_array, *numOfNodes, oldO + (float)(mapT->lastCmdSent).mp_turnDeg);
			}
			break;
		}
		case right_turn_ToRover:
		{
			if( (mapT->lastCmdSent).mp_turnDeg == 90 )
			{
				float oldO = getOrientation(map_array, *numOfNodes);
				(*numOfNodes)++;
				//printf("right Turn 90 num = %d", (*numOfNodes));
				setX(map_array, *numOfNodes, (mapT->curPose).x);
				setY(map_array, *numOfNodes, (mapT->curPose).y);
				setOrientation(map_array, *numOfNodes, oldO - (float)(mapT->lastCmdSent).mp_turnDeg);
			}
			break;
		}
		default:
		{
			break;
		}
	}
}

// update the rover pose using kinematics functions
void updateRoverPose(mapTStruct *mapT, webServerTaskStruct *webServerData, 
					rvPoseStruct *map_array, uint32_t *numOfNodes, 
					uint8_t rightFeet, uint8_t rightInches, uint8_t leftFeet, uint8_t leftInches)
{
	switch((mapT->lastCmdSent).mp_turn)
	{
		case (straight_turn_ToRover):
		{
			translateRoverVector((mapT->curPose).x, (mapT->curPose).y, (mapT->curPose).orientation
									, rightFeet, rightInches, leftFeet, leftInches);			
			(mapT->curPose).x = (resultOfRV_Translation.x >= 0) ? resultOfRV_Translation.x : 0;
			(mapT->curPose).y = (resultOfRV_Translation.y >= 0) ? resultOfRV_Translation.y : 0;
			(mapT->curPose).orientation = resultOfRV_Translation.orientation;
			uint16_t x = (uint16_t)((mapT->curPose).x/12);
			uint16_t y = (uint16_t)((mapT->curPose).y/12);
			mapT->distTravelled += convToInches(rightFeet, rightInches);
			SendWebServerData(webServerData, x, y, mapT->distTravelled, mapT->timeElapsedFromStart, 0);
			printDebug(mapT);
			break;
		}
		case (left_turn_ToRover):
		{			
			int leftF = (int)leftFeet;
			leftF = ((int)-1)*leftF;
			int leftI = (int)leftInches;
			leftI = ((int)-1)*leftI;
			//printf("leftF = %d, leftI = %d\n", leftF, leftI);
			translateRoverVector((mapT->curPose).x, (mapT->curPose).y, (mapT->curPose).orientation
									, rightFeet, rightInches, leftF, leftI);
			(mapT->curPose).x = resultOfRV_Translation.x;
			(mapT->curPose).y = resultOfRV_Translation.y;
			(mapT->curPose).orientation = resultOfRV_Translation.orientation;
			break;
		}
		case (right_turn_ToRover):
		{
			//printf("leftF = %d, leftI = %d\n", leftFeet, leftInches);
			translateRoverVector((mapT->curPose).x, (mapT->curPose).y, (mapT->curPose).orientation
									, (-1)*((int)rightFeet), (-1)*((int)rightInches), ((int)leftFeet), ((int)leftInches));
			(mapT->curPose).x = resultOfRV_Translation.x;
			(mapT->curPose).y = resultOfRV_Translation.y;
			(mapT->curPose).orientation = resultOfRV_Translation.orientation;
			//printDebug(mapT);
			break;
		}
		default:
		{
			break;
		}
	}
} 
// This is the actual task that is run
static portTASK_FUNCTION( mapTask, pvParameters )
{
	// Get the parameters
	mapTStruct *mapTPtr = (mapTStruct *) pvParameters;

	// Buffer for receiving messages
	mapTMsg msgBuf;	
	
	//Data structure for the webserver
	webServerTaskStruct *webServerData = (webServerTaskStruct *) mapTPtr->webServerData;
	//SendWebServerSensorData(webServerData, 30, 20, 20, portMAX_DELAY);

	//commandType cmd = {};
	//turnType t = straight;
	//SendWebServerCommandIssued(webServerData, cmd, t, 0, 12, 0, portMAX_DELAY);
   	setX(mapTPtr->map_array, mapTPtr->numOfNodesInMap, 0);
	setY(mapTPtr->map_array, mapTPtr->numOfNodesInMap, 0);
	setOrientation(mapTPtr->map_array, mapTPtr->numOfNodesInMap, 0.0);
	// Like all good tasks, this should never exit
	for(;;)
	{
		// Wait for a timer or a command message
		if (xQueueReceive(mapTPtr->inQ,(void *) &msgBuf,portMAX_DELAY) != pdTRUE) {
			VT_HANDLE_FATAL_ERROR(0);
		}

		switch( msgBuf.msgType )
		{
			case WheelDistance_ToMAP:
			{
#if smoothenMap == 1 
				if((mapTPtr->rovState == run_1 || mapTPtr->rovState == run_2) && ((mapTPtr->lastCmdSent).mp_turnDeg == 90 || 
																					(mapTPtr->lastCmdSent).mp_turnDeg == 0))
				{
				
					updateRoverPose(mapTPtr, webServerData, mapTPtr->map_array, 
									&(mapTPtr->numOfNodesInMap), msgBuf.rightFeet, 
										msgBuf.rightInches, msgBuf.leftFeet, msgBuf.leftInches);
				}
#else
				if((mapTPtr->rovState == run_1 || mapTPtr->rovState == run_2))
				{
				
					updateRoverPose(mapTPtr, webServerData, mapTPtr->map_array, 
									&(mapTPtr->numOfNodesInMap), msgBuf.rightFeet, 
										msgBuf.rightInches, msgBuf.leftFeet, msgBuf.leftInches);
				}
#endif
				break;
			}
			case Command_ToMAP:
			{
				if(mapTPtr->rovState == run_1 || mapTPtr->rovState == run_2)
				{ 
					(mapTPtr->lastCmdSent).mp_command = msgBuf.to_mp_command;
					(mapTPtr->lastCmdSent).mp_turn = msgBuf.to_mp_turn;
					(mapTPtr->lastCmdSent).mp_turnDeg = msgBuf.to_mp_turnDeg;
					updateMap(mapTPtr->map_array, &(mapTPtr)->numOfNodesInMap, mapTPtr);
				}				
				break;
			}
			case StFin_ToMAP:
			{
				int i = 0;
				for( i = 0; i < mapTPtr->numOfNodesInMap; i++ )
				{
					printf("X = %d, Y = %d\n", (mapTPtr->map_array[i]).x, (mapTPtr->map_array[i]).y);
					printf("Orientation = %d\n", (int)((double)(mapTPtr->map_array[i]).orientation));
				}
				switch(mapTPtr->rovState)
				{
					case undefined:
					{
						mapTPtr->numOfNodesInMap = 1;
						mapTPtr->rovState = run_1;
						break;
					}
					case run_1:
					{
						mapTPtr->segmentInMap = 1;
						(mapTPtr->curPose).x =( mapTPtr->map_array[mapTPtr->segmentInMap-1]).x;
						(mapTPtr->curPose).y =( mapTPtr->map_array[mapTPtr->segmentInMap-1]).y;
						(mapTPtr->curPose).orientation =( mapTPtr->map_array[mapTPtr->segmentInMap-1]).orientation;
						ClearWebServerGrid(webServerData, 0);
						mapTPtr->rovState = run_2;
						break;
					}
					case run_2:
					{						
						mapTPtr->rovState = done;
						break;
					}
					case done:
					{
						break;
					}
				}				
				break;
			}
			case timer_ToMAP:
			{
				mapTPtr->timeElapsedFromStart++;				
				break;
			}
			case prob_ToMAP:
			{
				mapTPtr->rovState = done;
				break;
			}
			default:
			{
				VT_HANDLE_FATAL_ERROR(0);
			}
		}
	}
}