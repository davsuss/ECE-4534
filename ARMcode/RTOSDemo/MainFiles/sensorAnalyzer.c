#include "sensorAnalyzer.h"
#include "RoverMessageTypes.h"
#include "rovermessageparser.h"
#include "communicationConstants.h"


/* *********************************************** */
// definitions and data structures that are private to this file

// I have set this to a large stack size because of (a) using printf() and (b) the depth of function calls
//   for some of the i2c operations	-- almost certainly too large, see LCDTask.c for details on how to check the size
#define INSPECT_STACK 1
#define baseStack 4
#if PRINTF_VERSION == 1
#define sensorSTACK_SIZE		((baseStack+5)*configMINIMAL_STACK_SIZE)
#else
#define sensorSTACK_SIZE		(baseStack*configMINIMAL_STACK_SIZE)
#endif
// end of defs
/* *********************************************** */

#include "debugPins.h"
bool pinState_respRcvd;
uint32_t pinMask_respRcvd = 0x40000; // pin 18

/* The sensor analyzer task. */
static portTASK_FUNCTION_PROTO( sensorTask, pvParameters );

#define MaxDeviationInches      2

//Max High Deviation for obstacle avoidance
#define MaxHighDeviation        7

#define SideSensorThreshold     30

#define OneUnit		2

char tempbuf[350];
char tempbuf2[350];

uint8_t count = 0;
uint8_t tiltCount = 0;

uint8_t RUN = 0;
uint8_t isDistanceFromMapValid = 1;

uint8_t statePrintedToWebServer = GLOBAL_READ;

/*-----------------------------------------------------------*/
// Public API
void startSensorTask(SensorAStruct *sensorT, unsigned portBASE_TYPE uxPriority, vtI2CStruct *i2c, MoveTaskStruct *moveT, mapTStruct *mapT, unsigned int unit_distance)
{
	/* Start the task */
	portBASE_TYPE retval;
	sensorT->i2cDev = i2c;
	sensorT->moveT = moveT;
	sensorT->mapT = mapT;
	sensorT->unit_distance = unit_distance;
    
	if ((retval =
         xTaskCreate( sensorTask,
                     ( signed char * ) "Sensor Analyzer", sensorSTACK_SIZE, (void *) sensorT,
                     uxPriority, ( xTaskHandle * ) NULL )) != pdPASS) {
             VT_HANDLE_FATAL_ERROR(retval);
         }
	// initialize pin and set direction
	pinState_respRcvd = initPin();
	GPIO_SetDir(0, pinMask_respRcvd, 1);
}

// End of Public API
/*-----------------------------------------------------------*/
// analyze sensor data and send commands
__INLINE void analyzeSensorDat(uint16_t *dat, MoveTaskStruct *moveT, mapTStruct *mapT, uint8_t num)
{
    //	commandType cmd[] = {start};
    //	turnType t[] = {straight};
    //	uint8_t deg[] = {0};
    //	uint8_t dist[] = {20, 20};
	
    //	SendMoveCommand(moveT, cmd, t, deg, dist, 1, 0);
    //	testMap(mapT, 0);
}

/*
 * Private function call to find out whether the side sensors
 * are within a specified tolerance. The differene between the
 * two side sensors values must be less then the defined max
 * deviation "MaxDeviationInches"
 *
 * Args:
 *  uint16_t - distance in inches read by the top right sensor
 *  uint16_t - distance in inches read by the bottom right sensor
 *
 *  Return:
 *  1 if the side sensor are within the deviation otherwise 0.
 */
uint8_t sideSensorsWithinTolerance(uint16_t topRight, uint16_t bottomRight)
{
    if (abs(topRight - bottomRight) >= MaxDeviationInches)
        return 0;
    else
        return 1;
}

/*
 * Private function call to find out whether both the side sensors
 * are reading a value in distance that is very large. This indicates
 * whether a right turn needs to be made ie the obstacle is cleared.
 *
 * Args:
 *  uint16_t topRight- distance in inches read by the top right sensor
 *  uint16_t bottomRight - distance in inches read by the bottom right sensor
 *
 * Return:
 *  1 if the side seonsor values are larger than the specified threshold otherwise 0.
 */
uint8_t sideSensorOutOfRange(uint16_t topRight, uint16_t bottomRight)
{
    return (topRight > SideSensorThreshold) && (bottomRight > SideSensorThreshold);
}

/*
 * Private function call to find out whether the side sensors
 * are within a specified high tolerance. The differene between the
 * two side sensors values must be less then the defined max
 * deviation "MaxDeviationInches"
 *
 * Args:
 *  uint16_t - distance in inches read by the top right sensor
 *  uint16_t - distance in inches read by the bottom right sensor
 *
 *  Return:
 *  1 if the side sensor are within the defined high deviation otherwise 0.
 */
uint8_t sideSensorWithinHighTolerance(uint16_t topRight, uint16_t bottomRight)
{
    if (abs(topRight - bottomRight) >= MaxHighDeviation)
        return 0;
    else
        return 1;
}

/*
 * Private function call to find out whether the rover is within
 * 3 feet from the wall.
 *
 * Args:
 *  uint16_t - distance in inches read by the top right sensor
 *  uint16_t - distance in inches read by the bottom right sensor
 *
 * Return:
 *  1 if the maximum of the two distances is bigger then 2.5 feet
 *  otherwise 0
 */
uint8_t roverWithin3Feet(uint16_t topRight, uint16_t bottomRight)
{
    return (topRight < 21) && (bottomRight < 21);
}

/*
 * Private functional call to find out whether the rover is 1 feet
 * from wall
 *
 * Args:
 *  uint16_t topRight - distance in inches read by the top right sensor
 *  uint16_t bottomRight - distance in inches read by the bottom right sensor
 *
 * Return:
 *  1 if rover is one feet from wall other wise 0
 */
uint8_t roverWithin1Feet(uint16_t topRight, uint16_t bottomRight)
{
    if (topRight > bottomRight)
        return (topRight <= 12);
    else
        return (bottomRight <= 12);
}

/*
 * Private function call to find out whether the rover is within
 * 3 feet from the wall.
 *
 * Args:
 *  uint16_t - distance in inches read by the top right sensor
 *  uint16_t - distance in inches read by the bottom right sensor
 *
 * Return:
 *  1 if the maximum of the two distances is lower then 2 feet
 *  otherwise 0
 */
uint8_t roverWithin2Feet(uint16_t topRight, uint16_t bottomRight)
{
    return (topRight < 18) && (bottomRight < 18);
}

/*
 * Private function call to find out whether the front sensor
 * is clear. Meaning that whether it is correct to move forward
 * a predefined unit step.
 *
 * Args:
 *  uint16_t - distance in inches read by the front sensor
 *
 * Return:
 *  1 if the front distance is signficantly greater than the pre-
 *  defined uint distance otherwise 0.
 */
uint8_t frontSensorClear(uint16_t frontDistanceInches)
{
    if ( frontDistanceInches < 24 ) return 0;
	else return 1;
}

char *fillWebServerBuf(uint8_t commandType, uint8_t turnType, uint8_t turnDegrees,
                       uint8_t distanceFeet, uint8_t distanceInches)

{
    //		char buf[350];
    if (commandType == start_cmd_ToRover)
        sprintf(tempbuf2, "Cmd: START ,");
    else if (commandType == stop_cmd_ToRover)
        sprintf(tempbuf2, "Cmd: STOP ,");
    else if (commandType == slow_cmd_ToRover)
        sprintf(tempbuf2, "Cmd: SLOW ,");
    else
        sprintf(tempbuf2, "Command: fast ,");
    
    if (turnType == straight_turn_ToRover)
        strcat(tempbuf2, "Turn: STRAIGHT ,");
    else if (turnType == left_turn_ToRover)
        strcat(tempbuf2, "Turn: LEFT ,");
    else
        strcat(tempbuf2, "Turn: RIGHT ,");
    
    sprintf(tempbuf, "Turn Degrees: %d, Feet: %d , Inches: %d\n", turnDegrees, distanceFeet, distanceInches);
    
    strcat(tempbuf2, tempbuf);

	if (statePrintedToWebServer == GLOBAL_READ) {
        sprintf(tempbuf, " GLOBAL READ \n");
    }
    
    else if (statePrintedToWebServer == GETTING_IN_3_FEET){
        sprintf(tempbuf, " Get In 3 Feet \n");
    }
    
    else if (statePrintedToWebServer == AVOID_OBSTACLE_1){
        sprintf(tempbuf, " AVOID_OBSTACLE_1 \n");
    }
    
    else
        sprintf(tempbuf, " Invalid State \n");

	strcat(tempbuf2, tempbuf);
    
    return (&tempbuf2[0]);
}

uint8_t getOneFeetToWall(MoveTaskStruct *moveT, webServerTaskStruct *webServerData, mapTStruct *mapT, uint16_t frontDistance)
{
	if(frontDistance <= 15)
		return 0;
    
	uint8_t cmd[] = {slow_cmd_ToRover};
	uint8_t t[] = {straight_turn_ToRover};
	uint8_t deg[] = {0};
	uint8_t dist[] = {0, frontDistance - 15};
	uint8_t numCommands = 1;
    
	SendWebServerCommandBuf(webServerData, fillWebServerBuf(cmd[0], t[0], deg[0], dist[0], dist[1]), portMAX_DELAY);
	SendMoveCommand(moveT, cmd, t, deg, dist, numCommands, portMAX_DELAY);
	sendNewCommand(mapT, cmd[0], t[0], deg[0], 0);
	return 1;
    
    
}

/*
 * Private function call to send the rover move forward command.
 *
 */
void moveForwardOneUnit(MoveTaskStruct *moveT, mapTStruct *mapT, webServerTaskStruct *webServerData)
{
    
    uint8_t cmd[] = {slow_cmd_ToRover};
	uint8_t t[] = {straight_turn_ToRover};
	uint8_t deg[] = {0};
	uint8_t dist[] = {1, 0};
	uint8_t numCommands = 1;
    
	/*
     #define start_cmd_ToRover 0
     #define stop_cmd_ToRover 1
     #define slow_cmd_ToRover 2
     #define fast_cmd_ToRover 3
     //----------------------------------------
     //possible turn ids
     #define straight_turn_ToRover 0
     #define left_turn_ToRover 1
     #define right_turn_ToRover 2
     //----------------------------------------
     //possible msgIDs
     #define sensorReq_msgID_ToRover 0
     #define sesorReplyPoll_msgID_ToARMPic 1
     #define sendCommand_msgID_ToRover 2
     #define sendStop_msgID_ToRover 3
     */
    
	//SendWebServerCommandIssued(webServerData, slow_cmd_ToRover, straight_turn_ToRover, 0, 1, 0, portMAX_DELAY);
    
    //	if (cmd[0] == start_cmd_ToRover)
    //        sprintf(tempbuf, "Cmd: START ,");
    //    else if (cmd[0] == stop_cmd_ToRover)
    //        sprintf(tempbuf, "Cmd: STOP ,");
    //    else if (cmd[0] == slow_cmd_ToRover)
    //        sprintf(tempbuf, "Cmd: SLOW ,");
    //    else
    //        sprintf(tempbuf, "Command: fast ,");
    //
    //    //sprintf(wsMsg.buf, tempbuf);
    //
    //    if (t[0] == straight_turn_ToRover)
    //        strcat(tempbuf, "Turn: STRAIGHT ,");
    //    else if (t[0] == left_turn_ToRover)
    //        strcat(tempbuf, "Turn: LEFT ,");
    //    else
    //        strcat(tempbuf, "Turn: RIGHT ,");
    //
    ////    strcat(wsMsg.buf, tempbuf);
    ////
    //    sprintf(tempbuf2, "Turn Degrees: %d, Feet: %d , Inches: %d\n", deg[0], dist[0], dist[1]);
    //
    //	strcat(tempbuf, tempbuf2);
    
	SendWebServerCommandBuf(webServerData, fillWebServerBuf(cmd[0], t[0], deg[0], dist[0], dist[1]), portMAX_DELAY);
    
    //	printf("cmd %d\n", cmd[0]);
    //	printf("turn %d\n", t[0]);
	/*
     (MoveTaskStruct *moveT,
     uint8_t *commandsToSend, // first block in communicationConstants.h
     uint8_t *turnIDs, 		// second block in communicationConstants.h
     uint8_t *degreeOfTurns,
     uint8_t *distances,
     uint8_t numOfCmds,
     portTickType ticksToBlock);
     */
	SendMoveCommand(moveT, cmd, t, deg, dist, numCommands, 0);
	sendNewCommand(mapT, cmd[0], t[0], deg[0], 0);
    
    //Need to send this information
	//testMap(mapT, 0);
}

void moveForward(MoveTaskStruct *moveT, mapTStruct *mapT, webServerTaskStruct *webServerData, uint16_t distanceInches)
{
    uint8_t cmd[] = {slow_cmd_ToRover};
	uint8_t t[] = {straight_turn_ToRover};
	uint8_t deg[] = {0};
	uint8_t dist[] = {0, distanceInches};
	uint8_t numCommands = 1;
    
    SendWebServerCommandBuf(webServerData, fillWebServerBuf(cmd[0], t[0], deg[0], dist[0], dist[1]), portMAX_DELAY);
    SendMoveCommand(moveT, cmd, t, deg, dist, numCommands, 0);
	sendNewCommand(mapT, cmd[0], t[0], deg[0], 0);
}

/*
 * Private function call to make a left turn by 90 degrees
 */
void leftTurn(MoveTaskStruct *moveT, mapTStruct *mapT, webServerTaskStruct *webServerData)
{
    /*
     #define start_cmd_ToRover 0
     #define stop_cmd_ToRover 1
     #define slow_cmd_ToRover 2
     #define fast_cmd_ToRover 3
     //----------------------------------------
     //possible turn ids
     #define straight_turn_ToRover 0
     #define left_turn_ToRover 1
     #define right_turn_ToRover 2
     //----------------------------------------
     //possible msgIDs
     #define sensorReq_msgID_ToRover 0
     #define sesorReplyPoll_msgID_ToARMPic 1
     #define sendCommand_msgID_ToRover 2
     #define sendStop_msgID_ToRover 3
     */
    
    uint8_t cmd[] = {slow_cmd_ToRover};
    uint8_t t[] = {left_turn_ToRover};
    uint8_t deg[] = {90};
    uint8_t dist[] = {0, 0};
	
	SendWebServerCommandBuf(webServerData, fillWebServerBuf(cmd[0], t[0], deg[0], dist[0], dist[1]), portMAX_DELAY);
    // private function to send commands - need to pass in buffer of commands
    // [cmd1, cmd2], [type1, type2], [turn1, turn2], [dist_feet1, dist_inch1, dist_feet2, dist_inches2], numCmds = 2
    
	SendMoveCommand(moveT, cmd, t, deg, dist, 1, portMAX_DELAY);
	sendNewCommand(mapT, cmd[0], t[0], deg[0], 0);
    //SendWebServerCommandIssued(webServerData, slow_cmd_ToRover, left_turn_ToRover, deg[0], dist[0], dist[1], 0);
    //Need to send this information
	//testMap(mapT, 0);
}

void rightTurn(MoveTaskStruct *moveT, mapTStruct *mapT, webServerTaskStruct *webServerData)
{
    /*
     #define start_cmd_ToRover 0
     #define stop_cmd_ToRover 1
     #define slow_cmd_ToRover 2
     #define fast_cmd_ToRover 3
     //----------------------------------------
     //possible turn ids
     #define straight_turn_ToRover 0
     #define left_turn_ToRover 1
     #define right_turn_ToRover 2
     //----------------------------------------
     //possible msgIDs
     #define sensorReq_msgID_ToRover 0
     #define sesorReplyPoll_msgID_ToARMPic 1
     #define sendCommand_msgID_ToRover 2
     #define sendStop_msgID_ToRover 3
     */
    
    uint8_t cmd[] = {slow_cmd_ToRover};
    uint8_t t[] = {right_turn_ToRover};
    uint8_t deg[] = {90};
    uint8_t dist[] = {0, 0};
	
	SendWebServerCommandBuf(webServerData, fillWebServerBuf(cmd[0], t[0], deg[0], dist[0], dist[1]), portMAX_DELAY);
    // private function to send commands - need to pass in buffer of commands
    // [cmd1, cmd2], [type1, type2], [turn1, turn2], [dist_feet1, dist_inch1, dist_feet2, dist_inches2], numCmds = 2
    
	SendMoveCommand(moveT, cmd, t, deg, dist, 1, portMAX_DELAY);
	sendNewCommand(mapT, cmd[0], t[0], deg[0], 0);
    //SendWebServerCommandIssued(webServerData, slow_cmd_ToRover, left_turn_ToRover, deg[0], dist[0], dist[1], 0);
    //Need to send this information
	//testMap(mapT, 0);
}

/*
 * Private function call to tilt the rover to the right
 * by specified degrees.
 *
 * Args:
 *  MoveTaskStruct *moveT - pointer to a data structure of type MoveTaskStruct
 *  mapTStruct *mapT - pointer to a data structure of type mapTStruct
 *  uint8_t degrees - number of degrees u want to tilt right.
 *  turnType
 *
 */
void tiltRover(MoveTaskStruct *moveT, mapTStruct *mapT, webServerTaskStruct *webServerData, uint8_t degrees, uint8_t turnDirection)
{
    /*
     #define start_cmd_ToRover 0
     #define stop_cmd_ToRover 1
     #define slow_cmd_ToRover 2
     #define fast_cmd_ToRover 3
     //----------------------------------------
     //possible turn ids
     #define straight_turn_ToRover 0
     #define left_turn_ToRover 1
     #define right_turn_ToRover 2
     //----------------------------------------
     //possible msgIDs
     #define sensorReq_msgID_ToRover 0
     #define sesorReplyPoll_msgID_ToARMPic 1
     #define sendCommand_msgID_ToRover 2
     #define sendStop_msgID_ToRover 3
     */
    
    uint8_t cmd[] = {slow_cmd_ToRover};
    uint8_t t[] = {turnDirection};
    uint8_t deg[] = {degrees};
    uint8_t dist[] = {0, 0};

	tiltCount++;
	
    // private function to send commands - need to pass in buffer of commands
    // [cmd1, cmd2], [type1, type2], [turn1, turn2], [dist_feet1, dist_inch1, dist_feet2, dist_inches2], numCmds = 2
    
    SendMoveCommand(moveT, cmd, t, deg, dist, 1, 0);
    //SendWebServerCommandIssued(webServerData, cmd[0], t[0], deg[0], dist[0], dist[1], portMAX_DELAY);
	SendWebServerCommandBuf(webServerData, fillWebServerBuf(cmd[0], t[0], deg[0], dist[0], dist[1]), portMAX_DELAY);
	sendNewCommand(mapT, cmd[0], t[0], deg[0], 0);
    
    //Need to send this information
	//testMap(mapT, 0);
}

// enum {GLOBAL_READ, GETTING_IN_3_FEET, AVOID_OBSTACLE_1};

void performLogic(uint8_t buffer [] , uint8_t length, MoveTaskStruct *moveTPtr, SensorAStruct *sensorT)
{
    webServerTaskStruct *webServerData = sensorT->webServerData;
    
    uint16_t frontDistanceInches = getFrontSensorDistanceInInches(buffer, length);
    uint16_t topRightDistanceInches = getTopRightSensorDistanceInInches(buffer, length);
    uint16_t bottomRightDistanceInches = getBottomRightSensorDistanceInInches(buffer, length);

	statePrintedToWebServer = current_state;
    
    if (current_state == GLOBAL_READ) {
        printf("GLOBAL READ \n");
    }
    
    else if (current_state == GETTING_IN_3_FEET){
        printf("Get In 3 Feet \n");
    }
    
    else if (current_state == AVOID_OBSTACLE_1){
        printf("AVOID_OBSTACLE_1 \n");
    }
    
    else
        printf("Invalid State \n");
    
    switch (current_state) {
            
        case GLOBAL_READ:
        {
            
            //Check whether both sensor values out of threshold
            
            uint8_t outOfRange = sideSensorOutOfRange(topRightDistanceInches, bottomRightDistanceInches);
            
			//printf("OutOfRange %d\n" , outOfRange);
            
            if (outOfRange == 1) {
                // Turn right and go to the state where u wait for the side
                // sensor values to be in range or the front is not clear
                
                if (count == 0) {
                    moveForward(moveTPtr, sensorT->mapT,webServerData, 8);
                    count++;
                    return;
                }
                
                if (prev_state == AVOID_OBSTACLE_1) {
                    prev_state = GLOBAL_READ;
                    current_state = GETTING_IN_3_FEET;
                }
                
                else {
                    
                    prev_state = GLOBAL_READ;
                    current_state = AVOID_OBSTACLE_1;
                }
                count = 0;
                rightTurn(moveTPtr, sensorT->mapT,webServerData);
                return;
            }
            
            
            //Check whether both sensor values are highly deviated
            
            uint8_t inHighTolerance = sideSensorWithinHighTolerance(topRightDistanceInches, bottomRightDistanceInches);
            
            // |  |-----------
            // |  |
            // |  |----
            
            //Rover not in high tolerance (posibly crossing the obstacle
            if (inHighTolerance == 0) {
                // Move a little forward and go to the state where u wait for the side
                // sensor values to be in range or the front is not clear
                
                uint8_t isFrontClear = frontSensorClear(frontDistanceInches);
                
                if (isFrontClear == 1)
                {
                    //printf("Move Forward\n");
                    moveForward(moveTPtr, sensorT->mapT,webServerData, 12);
                    return;
                }
                
                else if (getOneFeetToWall(moveTPtr, webServerData, sensorT->mapT, frontDistanceInches))
                {
                    return;
                }
                
                else {
                    //printf("Left Turn\n");
                    leftTurn(moveTPtr, sensorT->mapT,webServerData);
                }
                
                return;
                
            }
            
            // Check Whether the sensor values are low deviated
            
            uint8_t isInTolerance= sideSensorsWithinTolerance(topRightDistanceInches,
                                                              bottomRightDistanceInches);
            
            if (isInTolerance == 0 && tiltCount <= 3)
            {
                if (topRightDistanceInches > bottomRightDistanceInches)
                    tiltRover(moveTPtr, sensorT->mapT,webServerData, 10, right_turn_ToRover);
                else
                    tiltRover(moveTPtr, sensorT->mapT,webServerData, 10, left_turn_ToRover);
                return;
            }
           	
			tiltCount = 0;
            // Then check if the rover is within 3 feet
            
            uint8_t in3Feet = roverWithin3Feet(topRightDistanceInches, bottomRightDistanceInches);
            
            if (in3Feet == 0) {
                rightTurn(moveTPtr, sensorT->mapT,webServerData);
                prev_state = GLOBAL_READ;
                current_state = GETTING_IN_3_FEET;
                return;
            }

//			if (topRightDistanceInches <= 9 && bottomRightDistanceInches <= 9){
//				 leftTurn(moveTPtr, sensorT->mapT,webServerData);
//				 return;
//			}
            
            
            uint8_t isFrontClear = frontSensorClear(frontDistanceInches);
            
            if (isFrontClear == 1)
            {
                //printf("Move Forward\n");
				if ( RUN == 2 )
				{
					if ( frontDistanceInches > clearDistance(sensorT->mapT, 0) && isDistanceFromMapValid == 1)
					{
					   moveForward(moveTPtr, sensorT->mapT,webServerData, clearDistance(sensorT->mapT, 0));
					}

					else
					{
						moveForwardOneUnit(moveTPtr, sensorT->mapT,webServerData);
						isDistanceFromMapValid = 0;
					}
				}

				else
				{
                	moveForwardOneUnit(moveTPtr, sensorT->mapT,webServerData);
				}
            }
            
            else if (getOneFeetToWall(moveTPtr, webServerData, sensorT->mapT, frontDistanceInches))
            {
                return;
            }
            
            else {
                
                //printf("Left Turn\n");
                leftTurn(moveTPtr, sensorT->mapT,webServerData);
            }
            
            return;
        };
            
        case GETTING_IN_3_FEET:
        {
            
            uint8_t inRange = roverWithin3Feet(topRightDistanceInches, topRightDistanceInches);
            
            if (inRange == 1) {
                prev_state = GETTING_IN_3_FEET;
                current_state = GLOBAL_READ;
                break;
            }
            
            uint8_t isFrontClear = frontSensorClear(frontDistanceInches);
            
            if (isFrontClear == 1)
            {
					//printf("Move Forward\n");
				if ( RUN == 2 )
				{
					if ( frontDistanceInches > clearDistance(sensorT->mapT, 0) && isDistanceFromMapValid == 1)
					{
					   moveForward(moveTPtr, sensorT->mapT,webServerData, clearDistance(sensorT->mapT, 0));
					}

					else
					{
						moveForwardOneUnit(moveTPtr, sensorT->mapT,webServerData);
						isDistanceFromMapValid = 0;
					}
				}

				else
				{
                	moveForwardOneUnit(moveTPtr, sensorT->mapT,webServerData);
				}                

                break;
            }
            
            else if (getOneFeetToWall(moveTPtr, webServerData, sensorT->mapT, frontDistanceInches))
            {
                break;
            }
            
            else {
                prev_state = GETTING_IN_3_FEET;
                current_state = GLOBAL_READ;
                leftTurn(moveTPtr, sensorT->mapT,webServerData);
                break;
            }
            
            break;
        };
            
        case AVOID_OBSTACLE_1:
        {
            
            // Check Whether the sensor values are low deviated
            
            uint8_t inRange = roverWithin3Feet(topRightDistanceInches, topRightDistanceInches);
            
            if (inRange == 1) {
                prev_state = AVOID_OBSTACLE_1;
                current_state = GLOBAL_READ;
                break;
            }
            
            uint8_t isFrontClear = frontSensorClear(frontDistanceInches);
            
            if (isFrontClear == 1)
            {
				moveForwardOneUnit(moveTPtr, sensorT->mapT,webServerData);
			}
            
            else if (getOneFeetToWall(moveTPtr, webServerData, sensorT->mapT, frontDistanceInches))
            {
                break;
            }
            
            else {
                //printf("Left Turn\n");
                prev_state = AVOID_OBSTACLE_1;
                current_state = GLOBAL_READ;
                leftTurn(moveTPtr, sensorT->mapT,webServerData);
            }
            
            break;
        };
            
        default:
            break;
    }
}



//take in a buffer of sensor values, a new value, and return 1 if can use sensor value and 0 if can't 
//and useVal is the value that should be used
uint8_t updateSensorBuf(uint16_t *buf, uint16_t newVal, uint16_t *useVal)
{
	uint16_t sum = 0;
	int i = 0;
	for( i = 0; i < numOfSensorToDebounce; i++ )
	{
		sum += buf[i];
		if(i < numOfSensorToDebounce - 1)
		{
			buf[i] = buf[i+1];
		}
		else
		{
			buf[i] = newVal; // push the new Value
		}
	}
	(*useVal) = sum / numOfSensorToDebounce;
	if(newVal < (*useVal)+acceptableTolerance || newVal > (*useVal)+acceptableTolerance)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

// This is the actual task that is run
static portTASK_FUNCTION( sensorTask, pvParameters )
{
	uint8_t rxLen, status; // not using status, and rxLen is always maxReceiveBytesForSpecificNumOfSensorSamples (define in .h)
	uint8_t buffer[vtI2CMLen]; // receive message
	uint8_t recvMsgType; // message type of the message put on I2C queue by moveTask (defined in I2CtaskMsgtypes)
    
	// Get the parameters
	SensorAStruct *sensorT = (SensorAStruct *) pvParameters;
    
    webServerTaskStruct *webServerData = sensorT->webServerData;
	// Get the I2C device pointer
	vtI2CStruct *i2cDevPtr = sensorT->i2cDev;
	MoveTaskStruct *moveTPtr = sensorT->moveT;
    
    
	uint8_t count = 0;

	uint16_t frontBuf[numOfSensorToDebounce] = {0, 0, 0};
	uint16_t sideTopBuf[numOfSensorToDebounce] = {0, 0, 0};
	uint16_t sideBottomBuf[numOfSensorToDebounce] = {0, 0, 0};

	int i = 0;
	for( i = 0; i < numOfSensorToDebounce; i++ )
	{
		frontBuf[i] = 0;
		sideTopBuf[i] = 0;
		sideBottomBuf[i] = 0;
	}
    
    
	// Like all good tasks, this should never exit
	for(;;)
	{
		// Wait for a message from an I2C operation
		if (vtI2CDeQ(i2cDevPtr, vtI2CMLen,buffer,&rxLen,&recvMsgType,&status) != pdTRUE) {
			VT_HANDLE_FATAL_ERROR(0);
		}
        
        //		printf("buffer %x\n", buffer[0]);
        //
        //		uint8_t index = 0;
        //		for (index = 0; index < rxLen; index++)
        //		{
        //		 	printf("buffer %x\n", buffer[index]);
        //		}
        
		switch(recvMsgType)
		{
			case (sensor1):
			{
				break;
			}
			case (sensor2):
			{
				if( rxLen != 0 )
				{
                    // make sure the check sum is ok
					//printf("valid = %d\n", validateMessage(buffer, rxLen));
					if( validateMessage(buffer, rxLen) )
					{
						switch( getMessageId(buffer, rxLen) ) // recognize the msgID
						{
							case RoverWaitingForCommand:
							{
                                
                                
                                /*
                                 * We need to disable the timer so that the arm doesn't
                                 * retrieve multiple sensor values when the rover is stationary.
                                 * It is important to enable the timer back again so that the
                                 * Arm receives distance values the rover has travelled from the
                                 * last poll.
                                 */
                                
                                /*
                                 * Retrieve the sensor distances
                                 */
                                uint16_t frontDistanceInches = getFrontSensorDistanceInInches(buffer, rxLen);
								uint16_t usableFront = 0;
                                uint16_t topRightDistanceInches = getTopRightSensorDistanceInInches(buffer, rxLen);
								uint16_t usableSideT = 0;
                                uint16_t bottomRightDistanceInches = getBottomRightSensorDistanceInInches(buffer, rxLen);
								uint16_t usableSideB = 0;

								uint8_t canFront = updateSensorBuf(frontBuf, frontDistanceInches, &usableFront);
								uint8_t canSideT = updateSensorBuf(sideTopBuf, topRightDistanceInches, &usableSideT);
								uint8_t canSideB = updateSensorBuf(sideBottomBuf, bottomRightDistanceInches, &usableSideB);

								uint8_t rightFeet = getRightFeet(buffer, rxLen);
								uint8_t rightInches = getRightInches(buffer, rxLen);
								uint8_t leftFeet = getLeftFeet(buffer, rxLen);								  
								uint8_t leftInches = getLeftInches(buffer, rxLen);

//								printf("rightF = %d, rightI = %d, leftF = %d, leftI = %d\n", rightFeet, rightInches, leftFeet
//										, leftInches);

								if ( !canFront && !canSideT && !canSideB){
									return;
								}


								sendWheelDistancesOnly(sensorT->mapT, leftFeet, leftInches, rightFeet, rightInches, 0);
                                
                               	printf("Front %d\n", frontDistanceInches);
                                printf("TR %d\n", topRightDistanceInches);
                                printf("BR %d\n", bottomRightDistanceInches);
                                
                                SendWebServerSensorData(webServerData, usableFront,
                                                        usableSideT,usableSideB, 0);
                                
                                
                                performLogic(buffer , rxLen, moveTPtr, sensorT);
                                
								break;
							}
							case RoverBusy:
							{
                                /*
                                 * Simply pass the buffer received from the rover to the MAP Task.
                                 */
                                
                                /*
                                 * Since the rover has received the command, we want
                                 * to enable the timer so that we poll rover to receive
                                 * the distance in feet and inches the rover has moved.
                                 */
                                
                                break;
                            };
                                
							case RoverHitStartFinshLine:
							{
                                /*
                                 * Simply pass the buffer received from the rover to the MAP Task.
                                 */
								
								uint8_t cmd [] = {stop_cmd_ToRover};
								uint8_t t [] = {0};
								uint8_t deg [] = {0};
								uint8_t dist [] = {0,0};
								SendWebServerCommandBuf(webServerData, fillWebServerBuf(cmd[0], t[0], deg[0], dist[0], dist[1]), portMAX_DELAY);
                                //SendStopToRover(moveTPtr, portMAX_DELAY);
								SendStFinNotification(sensorT->mapT, portMAX_DELAY);
								performLogic(buffer , rxLen, moveTPtr, sensorT);
								RUN++;
                                break;
                            };
                                
                            case RoverReceiveMoveCommand:
                            {
                                /*
                                 * Since the rover has received the command, we want
                                 * to enable the timer so that we poll rover to receive
                                 * the distance in feet and inches the rover has moved.
                                 */
                                break;
                            };
                                
							case MessageDrop:
							{
                                // msg wasn't received in time
								SendDroppedNotification(moveTPtr, 0);
								break;
							};
						}
					}
                    
					else
					{
						// notify the moveTask that a message was droped or went bad
						SendDroppedNotification(moveTPtr, 0);
					}
				}
				else
				{
                    // message dropped
					SendDroppedNotification(moveTPtr, 0);
				}
                
				break;
			}
			case (cmdSend):
			{
				break;
			}
                
			default:
			{
				//VT_HANDLE_FATAL_ERROR(0);
				break;
			}
		}
	}
}
