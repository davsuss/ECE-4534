#include "rovermessageparser.h"
#include "RoverMessageTypes.h"

/*
 Author: Arjun Passi
 */

/**
 * Public API call to find out whether the message
 * received from the rover is valid or not.
 *
 * Args:
 *  uint8_t buffer - Buffer containing the message from the rover
 *  uint8_t length - Length of the passed buffer
 *
 * Return:
 * unit8_t - 1 if message is valid otherwise 0
 */
uint8_t validateMessage(uint8_t buffer [], uint8_t length)
{
    uint8_t msgId = getMessageId(buffer, length);
    
    if  (msgId == MessageDrop)
        return 1;
    
    uint8_t len = buffer[0] & 0xF;
    uint32_t rollingSum = 0;
	uint32_t i;
    
	for( i = 0; i < len + 5; i++ ) {
        rollingSum += buffer[i];
	}
    
    uint8_t checksum = buffer[len + 5];
    
	uint8_t flag =  ( (rollingSum & 0xFF) == checksum ) ? (uint8_t)1 : (uint8_t)0;
    
	if (flag == 0 ) return 0;
	if (msgId != RoverWaitingForCommand ) return flag;
    
	uint16_t frontDistanceInches = getFrontSensorDistanceInInches(buffer, length);
    uint16_t topRightDistanceInches = getTopRightSensorDistanceInInches(buffer, length);
    uint16_t bottomRightDistanceInches = getBottomRightSensorDistanceInInches(buffer, length);
    
	return (frontDistanceInches != 0 ) && ( topRightDistanceInches != 0 ) && (bottomRightDistanceInches != 0 );
}

/**
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
uint8_t getMessageId(uint8_t buffer [], uint8_t length)
{
    if (length > 0) {
        return (buffer[0] & 0xF0) >> 4;
    }
    
    else
        return MessageDrop;
}

/**
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
uint8_t getNumOfSampleBytes(uint8_t buffer[], uint8_t length)
{
    uint8_t numSensorSampleBytes = buffer[0] & 0xF;
    return numSensorSampleBytes;
}

/**
 * Public function call to retrieve the average raw front sensor value
 *
 * Args:
 *  uint8_t buffer - Buffer containing the message from the rover
 *  uint8_t length - Length of the passed buffer
 *
 * Return:
 *  uint8_t - average raw front sensor value
 */
uint16_t getFrontSensorValue(uint8_t buffer [], uint8_t length)
{
    if (length <= 0) {
        return 0;
    }
    
    uint8_t numSensorSampleBytes = getNumOfSampleBytes(buffer, length);
    
    // Notice before using any on the API function calls
    // Validate message function should be made. Validate
    // message call should handle checking of number of
    // sensor samples is between 0 and 9 and is a multiple of 3.
    
    if (numSensorSampleBytes <=0 || numSensorSampleBytes > 12) {
        return 0;
    }
    
    uint16_t value = 0;
    
    uint8_t count = 0;
    uint8_t index = 0;
    for (count = 0; count < numSensorSampleBytes / 6; count++) {
        value += (buffer[1 + index] << 8) | buffer[2 + index];
        index = index + 6;
    }
    
    value = value / count;
    
    return value;
    
}

/**
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
uint16_t getFrontSensorDistanceInInches(uint8_t buffer [], uint8_t length)
{
    uint16_t sensorValue = getFrontSensorValue(buffer, length);
    
    if (sensorValue == 0) {
        return 0;
    }
    
    uint16_t distance = 0;
    double value = 0;
    
#if FrontSensorType == BlueIRSensor
    
    // Equation to convert the sensor value from Blue IR Sensor
    // to distance in inches
    value = ( sensorValue * 3.3 ) / 1023;
    value = pow(value, 1.09);
    value = round(22.474 / value);
    
#endif
    
#if FrontSensorType == RedIRSensor
    
    // Equation to convert the sensor value from Red IR Sensor
    // to distance in inches
    
    value = ( sensorValue * 3.3 ) / 1023;
    value = pow(value, 1.03);
    value = round(22.9116 / value);
    
#endif
    
#if FrontSensorType == YellowIRSensor
    
    // Equation to convert yellow IR sensor value to
    // distance in inches.
    
    value = (sensorValue * 3.3) / 1023;
    value = pow(value, 1.02);
    value = round(22.4879/ value);
    
#endif
    
#if FrontSensorType == UltraSonicSensor
    
    // Equation to convert ultra sonic sensor value to
    // distance in inches.
    
    value = (sensorValue * 3300) / 1023;
    value = round( value / 9.8 );
    
#endif
    
    distance = (uint16_t) value;
    
    return distance;
}

/**
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
uint16_t getTopRightSensorValue(uint8_t buffer [], uint8_t length)
{
    if (length <= 0) {
        return 0;
    }
    
    uint8_t numSensorSampleBytes = getNumOfSampleBytes(buffer, length);
    
    // Validate message function call should handle
    // Checking of number of sensor samples is between
    // 0 and 9 and is a multiple of 3
    
    if (numSensorSampleBytes <=0 || numSensorSampleBytes > 12) {
        return 0;
    }
    
    uint16_t value = 0;
    
    uint8_t count = 0;
    uint8_t index = 0;
    for (count = 0; count < numSensorSampleBytes / 6; count++) {
        value += (buffer[3 + index] << 8) | buffer[4 + index];
        index = index + 6;
    }
    
    value = value / count;
    
    return value;
}

/**
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
uint16_t getTopRightSensorDistanceInInches(uint8_t buffer [], uint8_t length)
{
    uint16_t sensorValue = getTopRightSensorValue(buffer, length);
    
    if (sensorValue == 0) {
        return 0;
    }
    
    uint16_t distance = 0;
    double value = 0;
    
#if TopRightSensorType == BlueIRSensor
    
    // Equation to convert the sensor value from Blue IR Sensor
    // to distance in inches
    value = ( sensorValue * 3.3 ) / 1023;
    value = pow(value, 1.09);
    value = round(22.474 / value);
    
#endif
    
#if TopRightSensorType == RedIRSensor
    
    // Equation to convert the sensor value from Red IR Sensor
    // to distance in inches
    value = ( sensorValue * 3.3 ) / 1023;
    value = pow(value, 1.03);
    value = round(22.9116 / value);
    
#endif
    
#if TopRightSensorType == YellowIRSensor
    
    // Equation to convert yellow IR sensor value to
    // distance in inches.
    
    value = (sensorValue * 3.3) / 1023;
    value = pow(value, 1.02);
    value = round(22.4879/ value);
    
#endif
    
#if TopRightSensorType == UltraSonicSensor
    
    // Equation to convert ultra sonic sensor value to
    // distance in inches.
    
    value = (sensorValue * 3300) / 1023;
    value = round( value / 9.8 );
    
#endif
    
#if TopRightSensorType == A_IRSensor
    
    // Equation to convert short IR A sensor value to
    // distance in inches.
    
    value = (sensorValue * 3.3) / 1023;
    value = pow( value , 1.23);
    value = 11.4547 / value;
    
#endif
    
#if TopRightSensorType == B_IRSensor
    
    // Equation to convert ultra sonic sensor value to
    // distance in inches.
    
    value = (sensorValue * 3.3) / 1023;
    value = pow( value , 1.22);
    value = 11.143/value;
    
#endif
    
    distance = (uint16_t) value;
    
    return distance;
}

/**
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
uint16_t getBottomRightSensorValue(uint8_t buffer [], uint8_t length)
{
    if (length <= 0) {
        return 0;
    }
    
    uint8_t numSensorSampleBytes = getNumOfSampleBytes(buffer, length);
    
    // Validate message function call should handle
    // Checking of number of sensor samples is between
    // 0 and 9 and is a multiple of 3
    
    if (numSensorSampleBytes <=0 || numSensorSampleBytes > 12) {
        return 0;
    }
    
    uint16_t value = 0;
    
    uint8_t count = 0;
    uint8_t index = 0;
    for (count = 0; count < numSensorSampleBytes / 6; count++) {
        value += (buffer[5 + index] << 8) | buffer[6 + index];
        index = index + 6;
    }
    
    value = value / count;
    
    return value;
}

/**
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
uint16_t getBottomRightSensorDistanceInInches(uint8_t buffer [], uint8_t length)
{
    uint16_t sensorValue = getBottomRightSensorValue(buffer, length);
    
    if (sensorValue == 0) {
        return 0;
    }
    
    uint16_t distance = 0;
    double value = 0;
    
#if BottomRightSensorType == BlueIRSensor
    
    // Equation to convert the sensor value from Blue IR Sensor
    // to distance in inches
    value = ( sensorValue * 3.3 ) / 1023;
    value = pow(value, 1.09);
    value = round(22.474 / value);
    
#endif
    
#if BottomRightSensorType == RedIRSensor
    
    // Equation to convert the sensor value from Red IR Sensor
    // to distance in inches
    
    value = ( sensorValue * 3.3 ) / 1023;
    value = pow(value, 1.03);
    value = round(22.9116 / value);
    
#endif
    
#if BottomRightSensorType == YellowIRSensor
    
    // Equation to convert yellow IR sensor value to
    // distance in inches.
    
    value = (sensorValue * 3.3) / 1023;
    value = pow(value, 1.02);
    value = round(22.4879/ value);
    
#endif
    
#if BottomRightSensorType == UltraSonicSensor
    
    // Equation to convert ultra sonic sensor value to
    // distance in inches.
    
    value = (sensorValue * 3300) / 1023;
    value = round( value / 9.8 );
    
#endif
    
#if TopRightSensorType == A_IRSensor
    
    // Equation to convert short IR A sensor value to
    // distance in inches.
    
    value = (sensorValue * 3.3) / 1023;
    value = pow( value , 1.23);
    value = 11.4547 / value;
    
#endif
    
#if TopRightSensorType == B_IRSensor
    
    // Equation to convert ultra sonic sensor value to
    // distance in inches.
    
    value = (sensorValue * 3.3) / 1023;
    value = pow( value , 1.22);
    value = 11.143/value;
    
#endif
    
    distance = (uint16_t) value;
    
    return distance;
}

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
uint8_t getDistanceFeet(uint8_t buffer [], uint8_t length)
{
    uint8_t numSensorSampleBytes = getNumOfSampleBytes(buffer, length);
    return buffer[numSensorSampleBytes + 1];
}

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
uint8_t getDistanceInches(uint8_t buffer [], uint8_t length)
{
    uint8_t numSensorSampleBytes = getNumOfSampleBytes(buffer, length);
    return buffer[numSensorSampleBytes + 2];
}

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
uint8_t getRightFeet(uint8_t buffer [], uint8_t length)
{
    uint8_t numSensorSampleBytes = getNumOfSampleBytes(buffer, length);
    return buffer[numSensorSampleBytes + 1];
}

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
uint8_t getRightInches(uint8_t buffer [], uint8_t length)
{
    uint8_t numSensorSampleBytes = getNumOfSampleBytes(buffer, length);
    return buffer[numSensorSampleBytes + 2];
}

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
uint8_t getLeftFeet(uint8_t buffer [], uint8_t length)
{
    uint8_t numSensorSampleBytes = getNumOfSampleBytes(buffer, length);
    return buffer[numSensorSampleBytes + 3];
}

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
uint8_t getLeftInches(uint8_t buffer [], uint8_t length)
{
    uint8_t numSensorSampleBytes = getNumOfSampleBytes(buffer, length);
    return buffer[numSensorSampleBytes + 4];
}
