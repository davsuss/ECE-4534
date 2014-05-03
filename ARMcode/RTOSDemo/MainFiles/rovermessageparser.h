#ifndef ROVERMESSAGEPARSER_H
#define ROVERMESSAGEPARSER_H

#include <stdint.h>     /* uint8_t, uint16_t*/
#include <math.h>       /* pow, round*/


#define BlueIRSensor                0
#define RedIRSensor                 1
#define UltraSonicSensor            2
#define	YellowIRSensor				3
#define A_IRSensor                  4
#define B_IRSensor                  5
#define FrontSensorType             YellowIRSensor
#define TopRightSensorType          BlueIRSensor
#define BottomRightSensorType       RedIRSensor

/*
 Author: Arjun Passi
 */

/*
 * Public API call to find out whether the message
 * received from the rover is valid or not. This API
 * call must be called everytime a new message is recieved
 * in order to use other API calls defined.
 *
 * Args:
 *  uint8_t buffer - Buffer containing the message from the rover
 *  uint8_t length - Length of the passed buffer
 *
 * Return:
 * unit8_t - 1 if message is valid otherwise 0
 */
uint8_t validateMessage(uint8_t buffer [], uint8_t length);

/*
 * Public function call to retrieve the message id
 * present in the message from the rover.
 *
 * Args:
 *  uint8_t buffer - Buffer containing the message from the rover
 *  uint8_t length - Length of the passed buffer
 *
 * Return:
 *  uint8_t - message id present in the buffer
 */
uint8_t getMessageId(uint8_t buffer [], uint8_t length);

/*
 * Public function call to retrieve the number of samples
 * present in the message from the rover.
 *
 * Args:
 *  uint8_t buffer - Buffer containing the message from the rover
 *  uint8_t length - Length of the passed buffer
 *
 * Return:
 *  uint8_t - number of sensor sample bytes
 */
uint8_t getNumOfSampleBytes(uint8_t buffer[], uint8_t length);

/*
 * Public function call to retrieve the average raw front sensor value
 *
 * Args:
 *  uint8_t buffer - Buffer containing the message from the rover
 *  uint8_t length - Length of the passed buffer
 *
 * Return:
 *  uint8_t - average raw front sensor value
 */
uint16_t getFrontSensorValue(uint8_t buffer [], uint8_t length);

/*
 * Public function call to retrieve the distance in inches
 * that is measured by the front sensor
 *
 * Args:
 *  uint8_t buffer - Buffer containing the message from the rover
 *  uint8_t length - Length of the passed buffer
 *
 * Return:
 *  uint8_t - distance in inches from the front sensor value
 */
uint16_t getFrontSensorDistanceInInches(uint8_t buffer [], uint8_t length);

/*
 * Public function call to retrieve the average raw top right
 * sensor value
 *
 * Args:
 *  uint8_t buffer - Buffer containing the message from the rover
 *  uint8_t length - Length of the passed buffer
 *
 * Return:
 *  uint8_t - average raw top right sensor value
 */
uint16_t getTopRightSensorValue(uint8_t buffer [], uint8_t length);

/*
 * Public function call to retrieve the distance in inches
 * that is measured by the top right sensor value.
 *
 * Args:
 *  uint8_t buffer - Buffer containing the message from the rover
 *  uint8_t length - Length of the passed buffer
 *
 * Return:
 *  uint8_t - distance in inches from the top right sensor
 */
uint16_t getTopRightSensorDistanceInInches(uint8_t buffer [], uint8_t length);

/*
 * Public function call to retrieve the average raw bottom right
 * sensor value
 *
 * Args:
 *  uint8_t buffer - Buffer containing the message from the rover
 *  uint8_t length - Length of the passed buffer
 *
 * Return:
 *  uint8_t - average raw bottom right sensor value
 */
uint16_t getBottomRightSensorValue(uint8_t buffer [], uint8_t length);

/*
 * Public function call to retrieve the distance in inches
 * that is measured by the bottom right sensor value
 *
 * Args:
 *  uint8_t buffer - Buffer containing the message from the rover
 *  uint8_t length - Length of the passed buffer
 *
 * Return:
 *  uint8_t - distance in inches from the bottom right sensor
 */
uint16_t getBottomRightSensorDistanceInInches(uint8_t buffer [], uint8_t length);

/*
 * Public function call to retrieve distance in feet present
 * in the message recieved from the rover.
 *
 * Args:
 *  uint8_t buffer - Buffer containing the message from the rover
 *  uint8_t length - length of the passed buffer
 *
 * Return:
 *  uint8_t - distance in feet the rover has travelled since the last poll
 */
uint8_t getDistanceFeet(uint8_t buffer [], uint8_t length);

/*
 * Public function call to retrieve distance in inches present
 * in the message recieved from the rover.
 *
 * Args:
 *  uint8_t buffer - Buffer containing the message from the rover
 *  uint8_t length - length of the passed buffer
 *
 * Return:
 *  uint8_t - distance in inches the rover has travelled since the last poll
 */
uint8_t getDistanceInches(uint8_t buffer [], uint8_t length);

/**
 * Public function call to retrieve distance in feet present
 * in the message recieved from the rover.
 *
 * Args:
 *  uint8_t buffer - Buffer containing the message from the rover
 *  uint8_t length - length of the passed buffer
 *
 * Return:
 *  uint8_t - distance in feet the rover has travelled since the last poll
 */
uint8_t getRightFeet(uint8_t buffer [], uint8_t length);

/**
 * Public function call to retrieve distance in inches present
 * in the message recieved from the rover.
 *
 * Args:
 *  uint8_t buffer - Buffer containing the message from the rover
 *  uint8_t length - length of the passed buffer
 *
 * Return:
 *  uint8_t - distance in inches the rover has travelled since the last poll
 */
uint8_t getRightInches(uint8_t buffer [], uint8_t length);

 /*
 * Public function call to retrieve distance in feet present
 * in the message recieved from the rover.
 *
 * Args:
 *  uint8_t buffer - Buffer containing the message from the rover
 *  uint8_t length - length of the passed buffer
 *
 * Return:
 *  uint8_t - distance in feet the rover has travelled since the last poll
 */
uint8_t getLeftFeet(uint8_t buffer [], uint8_t length);

/*
 * Public function call to retrieve distance in inches present
 * in the message recieved from the rover.
 *
 * Args:
 *  uint8_t buffer - Buffer containing the message from the rover
 *  uint8_t length - length of the passed buffer
 *
 * Return:
 *  uint8_t - distance in inches the rover has travelled since the last poll
 */
uint8_t getLeftInches(uint8_t buffer [], uint8_t length);

#endif