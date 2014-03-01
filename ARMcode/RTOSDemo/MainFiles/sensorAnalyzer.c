#include "sensorAnalyzer.h"


/* *********************************************** */
// definitions and data structures that are private to this file

// I have set this to a large stack size because of (a) using printf() and (b) the depth of function calls
//   for some of the i2c operations	-- almost certainly too large, see LCDTask.c for details on how to check the size
#define INSPECT_STACK 1
#define baseStack 2
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
	commandType cmd[] = {0};
	turnType t[] = {0};
	
	SendMoveCommand(moveT, cmd, t, 1, 0);
	testMap(mapT, 0);	
}

// check if the checkSum can be validated
uint8_t validateMsg(uint32_t len, uint8_t *buf, uint8_t checkSum)
{
	uint32_t rollingSum = 0;
	uint32_t i;
	for( i = 0; i < len; i++ )
	{
		 rollingSum += buf[i];
	}
	return ( (rollingSum & 0xFF) == checkSum ) ? (uint8_t)1 : (uint8_t)0;
}
 
// This is the actual task that is run
static portTASK_FUNCTION( sensorTask, pvParameters )
{
	uint8_t rxLen, status; // not using status, and rxLen is always maxReceiveBytesForSpecificNumOfSensorSamples (define in .h)
	uint8_t Buffer[vtI2CMLen]; // receive message
	uint16_t sensorDat[maxSensorSamples]; // sensor
	uint8_t recvMsgType; // message type of the message put on I2C queue by moveTask (defined in I2CtaskMsgtypes)	

	// Get the parameters
	SensorAStruct *sensorT = (SensorAStruct *) pvParameters;
	// Get the I2C device pointer
	vtI2CStruct *i2cDevPtr = sensorT->i2cDev;
	MoveTaskStruct *moveTPtr = sensorT->moveT;

	// Like all good tasks, this should never exit
	for(;;)
	{
		// Wait for a message from an I2C operation
		if (vtI2CDeQ(i2cDevPtr, vtI2CMLen,Buffer,&rxLen,&recvMsgType,&status) != pdTRUE) {
			VT_HANDLE_FATAL_ERROR(0);
		} 

		switch(recvMsgType)
		{
			case (sensor1):
			{
				break;
			}
			case (sensor2):
			{
				uint8_t i;				
				if( rxLen != 0 )
				{			
					if( validateMsg(maxReceiveBytesForSpecificNumOfSensorSamples - 1, 
									Buffer, 
									Buffer[maxReceiveBytesForSpecificNumOfSensorSamples - 1]) ) // make sure the check sum is ok 
					{
						switch( (Buffer[1] & 0xF0) >> 4 ) // recognize the msgID
						{
							case 0x0:
							{
								uint8_t count = Buffer[1] & 0x0F; // number of sensor samples
							
								for( i = 0; i < count; i+=2 )
								{
									sensorDat[i] = Buffer[(2 + i)] << 8 | Buffer[(2 + i) + 1]; // re build the 10 - bit sample
								}

								// send command according to the sensor values
								analyzeSensorDat(sensorDat, moveTPtr, sensorT->mapT, count);
								(pinState_respRcvd == True) ? (GPIO_ClearValue(0, pinMask_respRcvd)) : (GPIO_SetValue(0, pinMask_respRcvd));
								pinState_respRcvd = updatePin(pinState_respRcvd);							
								break;
							}
							case 0x1:
							{break;}
							case 0x2:
							{break;}
							case 0xF:
							{
								SendDroppedNotification(moveTPtr, 0); // msg wasn't received in time
								break;
							}
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
					SendDroppedNotification(moveTPtr, 0); // message dropped	
				}
				break;
			}
			case (cmdSend):
			{
				break;
			}
			default:
			{
				VT_HANDLE_FATAL_ERROR(0);
				break;
			}
		}
	}
}
