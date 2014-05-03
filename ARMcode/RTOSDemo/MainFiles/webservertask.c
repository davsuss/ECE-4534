/* include files. */
#include "moveTask.h"
#include "webservertask.h"
#include "webServerMessageTypes.h"
#include <stdlib.h>
#include <string.h>

// I have set this to a larger stack size because of (a) using printf() and (b) the depth of function calls
//   for some of the LCD operations
// I actually monitor the stack size in the code to check to make sure I'm not too close to overflowing the stack
//   This monitoring takes place if INPSECT_STACK is defined (search this file for INSPECT_STACK to see the code for this)
#define INSPECT_STACK 1
#define baseStack 3
#if PRINTF_VERSION == 1
#define webServerSTACK_SIZE		((baseStack+5)*configMINIMAL_STACK_SIZE)
#else
#define webServerSTACK_SIZE		(baseStack*configMINIMAL_STACK_SIZE)
#endif

#define webServerQLen 20

#if SENSOR_BUF_SIZE >= COMMAND_BUF_SIZE
#define maxWebServerBufLen SENSOR_BUF_SIZE
#endif

#if COMMAND_BUF_SIZE > SENSOR_BUF_SIZE
#define maxWebServerBufLen COMMAND_BUF_SIZE
#endif

MoveTaskStruct *moveTaskStruct;

/**
 * Defining a struct that will be sent in a message
 */
typedef struct __webServerMSG {
    uint8_t msgType;
    uint16_t xCoordinate;
    uint16_t yCoordinate;
    uint16_t distance;
    uint16_t time;
    char buf[maxWebServerBufLen];
} webServerMSG;

/* definition for the web server task. */
static portTASK_FUNCTION_PROTO( vWebServerTask, pvParameters );

/**
 * Function call to retrieve the message type
 *
 * Args:
 *  wsMsg - pointer to a variable of type webServerMSG struct.
 *
 * Return:
 *  Message type
 */
uint8_t getMsgType(webServerMSG *wsMsg)
{
	return(wsMsg->msgType);
}

/**
 * Public API call to start the webserver task.
 *
 * Args:
 *  webserverTaskStruct -- pointer to the variable of type webServerTaskStruct
 *  taskPriority -- the priority with which this task must be run
 */
void StartWebserverTask(webServerTaskStruct *webServerData, unsigned portBASE_TYPE taskPriority)
{
    if(webServerData == NULL){
        VT_HANDLE_FATAL_ERROR(0);
    }
    
    // Create the queue that will be used to talk to this task
	if ((webServerData->inQ = xQueueCreate(webServerQLen,sizeof(webServerMSG))) == NULL) {
		VT_HANDLE_FATAL_ERROR(0);
	}
    
    /* Start the task */
	portBASE_TYPE retval;
	if ((retval = xTaskCreate( vWebServerTask, ( signed char * ) "Web-Server", webServerSTACK_SIZE, (void*)webServerData, taskPriority, ( xTaskHandle * ) NULL )) != pdPASS) {
		VT_HANDLE_FATAL_ERROR(retval);
	}
}

/**
 * Public API call to send X and Y coordinates to the webserver task.
 *
 * Args:
 *  webServerData -- pointer to the variable of type webServerStruct
 *  xCoord -- Value of the X coordinate
 *  yCoor -- Value of the Y coordinate
 *  distance -- distance the rover has travelled
 *  time -- time the rover has been travelling
 *  ticksToBlock -- how long the routine should wait if the queue is full
 *
 * Return:
 *  result of the call to xQueueSend()
 */
portBASE_TYPE SendWebServerData(webServerTaskStruct *webServerData, uint16_t xCoord,
                                uint16_t yCoord, uint16_t distance, uint16_t time,
                                portTickType ticksToBlock)
{
    if (webServerData == NULL){
        VT_HANDLE_FATAL_ERROR(0);
    }
    
    webServerMSG wsMsg;
    wsMsg.msgType = drawPoint;
    wsMsg.xCoordinate = xCoord;
    wsMsg.yCoordinate = yCoord;
    wsMsg.distance = distance;
    wsMsg.time = time;
    
    return (xQueueSend(webServerData->inQ, (void *) (&wsMsg), ticksToBlock));
}

/**
 * Public API call to clear the grid on the web-server.
 *
 * Args:
 *  webServerData - pointer to the variable of type webServerStruct.
 *  ticksToBlock -- how long the routine should wait if the queue is full
 *
 * Return:
 * result of the call to xQueueSend()
 */
portBASE_TYPE ClearWebServerGrid(webServerTaskStruct *webServerData,
                                 portTickType ticksToBlock)
{
    if (webServerData == NULL){
        VT_HANDLE_FATAL_ERROR(0);
    }
    
    webServerMSG wsMsg;
    wsMsg.msgType = clearGrid;
    wsMsg.distance = 0;
    wsMsg.time = 0;
    
    return (xQueueSend(webServerData->inQ, (void *) (&wsMsg), ticksToBlock));
}

/**
 * Public API call to post the sensor data on the web-server.
 *
 * Args:
 *  webServerData - pointer to the variable of type webServerStruct.
 *  uint16_t frontDistance - Distance in inches read from the first sensor
 *  uint16_t topRightDistance - Distance in inches read from the top right sensor
 *  uint16_t bottomRightDistance - Distance in inches read from the bottom right sensor.
 *  ticksToBlock -- how long the routine should wait if the queue is full
 *
 * Return:
 * result of the call to xQueueSend()
 */
portBASE_TYPE SendWebServerSensorData(webServerTaskStruct *webServerData, uint16_t frontDistance,
                                      uint16_t topRightDistance, uint16_t bottomRightDistance,
                                      portTickType ticksToBlock)
{
    if (webServerData == NULL){
        VT_HANDLE_FATAL_ERROR(0);
    }
    
    webServerMSG wsMsg;
    wsMsg.msgType = sensorData;
    
    sprintf(wsMsg.buf, "Front Distance: %d , Top Right Distance: %d , Bottom Right Distance: %d\n",
            frontDistance, topRightDistance, bottomRightDistance);
   
   	//printf(wsMsg.buf);
    //printf("\n");
    
    return (xQueueSend(webServerData->inQ, (void *) (&wsMsg), ticksToBlock));
}

/**
 * Public API call to post the command issued by the arm on the web-server.
 *
 * Args:
 *  webServerData - pointer to the variable of type webServerStruct.
 *  uint8_t commandType - command type the rover issued
 *  uint8_t turnType - turnType the rover issued.
 *  uint8_t turnDegrees - degrees the rover was asked to turn
 *  uint8_t distanceFeet - distance in feet the rover was asked to travel
 *  uint8_t distanceInches - distance in inches the rover was asked to travel
 *  ticksToBlock -- how long the routine should wait if the queue is full
 *
 * Return:
 * result of the call to xQueueSend()
 */
portBASE_TYPE SendWebServerCommandIssued(webServerTaskStruct *webServerData, uint8_t commandType,
                                   uint8_t turnType, uint8_t turnDegrees, uint8_t distanceFeet,
                                   uint8_t distanceInches, portTickType ticksToBlock)
{
    if (webServerData == NULL){
        VT_HANDLE_FATAL_ERROR(0);
    }
    
    webServerMSG wsMsg;
    wsMsg.msgType = commandIssued;
    
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
    
//	printf("%d\n", distanceFeet);    

    if (commandType == start_cmd_ToRover)
        sprintf(wsMsg.buf, "Cmd: START ,");
    else if (commandType == stop_cmd_ToRover)
        sprintf(wsMsg.buf, "Cmd: STOP ,");
    else if (commandType == slow_cmd_ToRover)
        sprintf(wsMsg.buf, "Cmd: SLOW ,");
    else
        sprintf(wsMsg.buf, "Command: fast ,");
    
    //sprintf(wsMsg.buf, tempbuf);
    
    if (turnType == straight_turn_ToRover)
        strcat(wsMsg.buf, "Turn: STRAIGHT ,");
    else if (turnType == left_turn_ToRover)
        strcat(wsMsg.buf, "Turn: LEFT ,");
    else
        strcat(wsMsg.buf, "Turn: RIGHT ,");
//    
//    strcat(wsMsg.buf, tempbuf);
//    
//    sprintf(tempbuf, "Turn Degrees: %d, Feet: %d , Inches: %d\n",
//            turnDegrees, distanceFeet, distanceInches);
//    
//    strcat(wsMsg.buf, tempbuf);
//
//	printf("%s\n", wsMsg.buf);
    
    return (xQueueSend(webServerData->inQ, (void *) (&wsMsg), ticksToBlock));
}

portBASE_TYPE SendWebServerCommandBuf(webServerTaskStruct *webServerData, char buf[] ,portTickType ticksToBlock)
{
	if (webServerData == NULL){
        VT_HANDLE_FATAL_ERROR(0);
    }
    
    webServerMSG wsMsg;
    wsMsg.msgType = commandIssued;

	strcpy(wsMsg.buf, buf);

	return (xQueueSend(webServerData->inQ, (void *) (&wsMsg), ticksToBlock));
}

/*
 * Public API call to send the start command to the rover.
 * Thi function call should be used when the start button
 * is pressed on the webserver.
 *
 * Args:
 *  void
 *
 * Return:
 * result of the call to xQueueSend()
 */
portBASE_TYPE SendRoverStartCommand()
{
	SendStartToMvTask(moveTaskStruct, portMAX_DELAY);
}

/*
 * Public API call to send the stop command to the rover.
 * Thi function call should be used when the stop button
 * is pressed on the webserver.
 *
 * Args:
 *  - void
 *
 * Return:
 * result of the call to xQueueSend()
 */
portBASE_TYPE SendRoverStopCommand()
{
      SendStopToMvTask(moveTaskStruct, portMAX_DELAY);
}

/**
 * Public API calls to retrieve the formatted string containing
 * all the sensor information. This API call should be made use
 * of inorder to view the sensor value history on the webserver.
 *
 * Args:
 *  - void
 *
 * Return:
 *  char * - formatted string containing the sensor values.
 */
char * getSensorValueString(void)
{
    //Setting the flag high
    sensorBufferRead = 1;
    return (char *) (&sensorValuesBuf[0]);
}

/**
 * Public API calls to retrieve the formatted string containing
 * last command issued by ARM. This API call should be made use
 * of inorder to view the command history on the webserver.
 *
 * Args:
 *  - void
 *
 * Return:
 *  char [] - formatted string containing the last command.
 */
char * getCommandIssuedString(void)
{
    //Setting the flag high
    commandBufferRead = 1;
    return (char *) (&commandsIssuedBuf[0]);
}

/**
 * Public API call to retrieve whether the sensor buffer
 * was read by the webserver. This flag is essentially
 * set to 1/true if the webserver makes a call to the
 * getSensorValueString.
 *
 * Args:
 *  - void
 *
 * Return:
 *  0 if the new sensor string was not read by the webserver otherwise 1
 */
uint8_t sensorValueBufferRead(void)
{
	uint8_t prev = sensorBufferRead;
    return prev;
}

void setSensorValueFlag(uint8_t value)
{
	sensorBufferRead = value;
}

void setCommandIssueFlag(uint8_t value)
{
	commandBufferRead = value;
}

/**
 * Public API call to retrieve whether the command issued
 * buffer was read by the webserver. This flag is essentially
 * set to 1/true if the webserver makes a call to the
 * getCommandIssuedString.
 *
 * Args:
 *  - void
 *
 * Return:
 *  0 if the new command issued string was not read by the webserver otherwise 1
 */
uint8_t commandIssuedBufferRead(void)
{
	uint8_t prev = commandBufferRead;
    return prev;
}

/**
 * Public API call to retreive the X coordinate
 * at the index provided.
 *
 * Args:
 *  index - location of the X coordinate in the buffer.
 *
 * Return - x coordinate at the provided index.
 */
uint16_t getXCoordinate(uint16_t index)
{
    if (index >= 0 && index < numPoints)
        return xCoordinates[index];
    else
        return (uint16_t) 0;
}


/**
 * Public API call to retreive the Y coordinate
 * at the index provided.
 *
 * Args:
 *  index - location of the Y coordinate in the buffer.
 *
 * Return:
 *  Y coordinate at the provided index.
 */
uint16_t getYCoordinate(uint16_t index)
{
    if (index >= 0 && index < numPoints)
        return yCoordinates[index];
    else
        return (uint16_t) 0;
}


/**
 * Public API call to retreive the number of coordinates
 * the map task has sent.
 *
 * Args: void
 *
 * Return:
 *  uint16_t - total number of coordinates.
 */
uint16_t getNumCoordinates(void)
{
    return numPoints;
}

/**
 * Publid API call to retreive the distance the rover has travelled.
 *
 * Args: void
 *
 * Return:
 *  uint16_t - distance travelled by the rover
 */
uint16_t getDistance(void)
{
    return mDistanceTravelled;
}

/**
 * Public API call to retreive the max
 * X coordinate that was received. This
 * function call will be beneficial in resizing
 * the grid on the web-server
 *
 * Args: void
 *
 * Return:
 *  uint16_t Max X coordinate
 */
uint16_t getMaxXcoord(void)
{
    return maxXcoord;
}

/**
 * Public API call to retreive the max
 * Y coordinate that was received. This
 * function call will be beneficial in resizing
 * the grid on the web-server
 *
 * Args: void
 *
 * Return:
 *  uint16_t Max Y coordinate
 */
uint16_t getMaxYcoord(void)
{
    return maxYcoord;
}

/**
 * Public API call to retreive the time elapsed.
 *
 * Args: void
 *
 * Return:
 *  uint16_t - time elapsed
 */
uint16_t getTime(void)
{
    return mTimeElapsed;
}

static portTASK_FUNCTION( vWebServerTask, pvParameters )
{
    // Retreiving the webServerStruct
    webServerTaskStruct *wsTaskStruct = (webServerTaskStruct *) pvParameters;
    
    // Retreiving the moveTaskStruct
    moveTaskStruct = wsTaskStruct->moveTaskStruct;
    
    //Web Server Message
    webServerMSG wsMsg;

	sensorBufferRead = 1;
	commandBufferRead = 1;
    
    // Non-terminating task
    for(;;)
    {
        // Try to dequeue a message from the webServerTask inQ
        if (xQueueReceive(wsTaskStruct->inQ,(void *) &wsMsg,portMAX_DELAY) == pdTRUE) {
            
            switch (getMsgType(&wsMsg)) {
                case clearGrid:
                {
                    numPoints = 0;
                    maxXcoord = 0;
                    maxYcoord = 0;
                    mTimeElapsed = 0;
                    mDistanceTravelled = 0;
                    break;
                }
                    
                case drawPoint:
                {
                    if (numPoints >= MAX_POINTS)
                        numPoints = 0;
                    
                    xCoordinates[numPoints] = wsMsg.xCoordinate;
                    yCoordinates[numPoints] = wsMsg.yCoordinate;
                    
                    if (wsMsg.xCoordinate > maxXcoord)
                        maxXcoord = wsMsg.xCoordinate;
                    
                    if (wsMsg.yCoordinate > maxYcoord)
                        maxYcoord = wsMsg.yCoordinate;
                    
                    mDistanceTravelled = wsMsg.distance;
                    mTimeElapsed = wsMsg.time;
                    numPoints++;
                    break;
                }
                    
                case debugInfo:
                {
                    break;
                }
                    
                case sensorData:
                {
                    //Setting the flag low as
                    //a newer value is read.
					//while ( sensorBufferRead == 0 ) {};
                    sprintf(sensorValuesBuf, wsMsg.buf);
					sensorBufferRead = 0;
                    break;
                };
                    
                case commandIssued:
                {
                    //Setting the flag low as
                    //a newer value is read
					//while ( commandBufferRead == 0 ) {};
                    sprintf(commandsIssuedBuf, wsMsg.buf);
					commandBufferRead = 0;
                    break;
                };
                    
                default:
                    break;
            }
		}
    }
    
}