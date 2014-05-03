#ifndef WEBSERVERTASK_H
#define WEBSERVERTASK_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "projdefs.h"
#include "semphr.h"
#include "queue.h"

/* include files. */
#include "webservertask.h"
#include "vtUtilities.h"
#include "moveTask.h"

#define MAX_POINTS  200
#define SENSOR_BUF_SIZE 256
#define COMMAND_BUF_SIZE 256

// Variables to keep track of the distance
// travelled and time elapsed.
uint16_t mDistanceTravelled = 0;
uint16_t mTimeElapsed = 0;

uint16_t xCoordinates[MAX_POINTS];
uint16_t yCoordinates[MAX_POINTS];
uint16_t numPoints = 0;

uint16_t maxXcoord = 0;
uint16_t maxYcoord = 0;

char sensorValuesBuf[SENSOR_BUF_SIZE];
char commandsIssuedBuf[COMMAND_BUF_SIZE];
char tempbuf[350];

uint8_t sensorBufferRead;
uint8_t commandBufferRead;


/*
 Defining a struct that will be used to pass coordinates
 to the webserver task.
 */
typedef struct __webServerTaskStruct{
    
    /*
     * Reference to data structure of
     * type moveTaskStruc. Since the web
     * server task will provide API calls
     * to control the rover from the webserver
     * we keep reference to this struct.
     */
    MoveTaskStruct *moveTaskStruct;
    
    /*
     * Defining a queue that should
     * should be used to send messages
     * to the webserver task from any
     * other task.
     */
    xQueueHandle inQ;
} webServerTaskStruct;

//--------------------------------------------------------
// Following are the public API functions that other tasks
// should use to work with the webserver task.
//--------------------------------------------------------

/*
 * Public API call to start the webserver task.
 *
 * Args:
 *  webserverTaskStruct -- pointer to the variable of type webServerTaskStruct
 *  taskPriority -- the priority with which this task must be run
 */
void StartWebserverTask(webServerTaskStruct *webServerData, unsigned portBASE_TYPE taskPriority);

/*
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
                                portTickType ticksToBlock);

/*
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
                                 portTickType ticksToBlock);

/*
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
                                      portTickType ticksToBlock);

/*
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
portBASE_TYPE SendWebServerCommandIssued(webServerTaskStruct *webServerData, uint8_t commandTyp,
                                   uint8_t turnType, uint8_t turnDegrees, uint8_t distanceFeet,
                                   uint8_t distanceInches, portTickType ticksToBlock);

portBASE_TYPE SendWebServerCommandBuf(webServerTaskStruct *webServerData, char buf[], portTickType ticksToBlock);

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
portBASE_TYPE SendRoverStartCommand();

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
portBASE_TYPE SendRoverStopCommand();

/*
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
char * getSensorValueString(void);

/*
 * Public API calls to retrieve the formatted string containing
 * last command issued by ARM. This API call should be made use
 * of inorder to view the command history on the webserver.
 *
 * Args:
 *  - void
 *
 * Return:
 *  char * - formatted string containing the last command.
 */
char * getCommandIssuedString(void);

/*
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
uint8_t sensorValueBufferRead(void);

/*
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
uint8_t commandIssuedBufferRead(void);

void setSensorValueFlag(uint8_t value);

void setCommandIssueFlag(uint8_t value);

/*
 * Public API call to retreive the X coordinate
 * at the index provided.
 *
 * Args:
 *  index - location of the X coordinate in the buffer.
 *
 * Return - x coordinate at the provided index.
 */
uint16_t getXCoordinate(uint16_t index);


/*
 * Public API call to retreive the Y coordinate
 * at the index provided.
 *
 * Args:
 *  index - location of the Y coordinate in the buffer.
 *
 * Return:
 *  Y coordinate at the provided index.
 */
uint16_t getYCoordinate(uint16_t index);


/*
 * Public API call to retreive the number of coordinates
 * the map task has sent.
 *
 * Args: void
 *
 * Return:
 *  uint16_t - total number of coordinates.
 */
uint16_t getNumCoordinates(void);

/*
 * Publid API call to retreive the distance the rover has travelled.
 *
 * Args: void
 *
 * Return:
 *  uint16_t - distance travelled by the rover
 */
uint16_t getDistance(void);

/*
 * Public API call to retreive the time elapsed.
 *
 * Args: void
 *
 * Return:
 *  uint16_t - time elapsed
 */
uint16_t getTime(void);

/*
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
uint16_t getMaxXcoord(void);

/*
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
uint16_t getMaxYcoord(void);

#endif
