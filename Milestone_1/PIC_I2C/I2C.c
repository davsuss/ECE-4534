#include "maindefs.h"
#include <plib/timers.h>
#include <plib/i2c.h>
#include "user_interrupts.h"
#include "messages.h"
#include "debug.h"
#include "I2C.h"
#define TEST 2
typedef enum {I2C_IDLE,I2C_REQUEST,I2C_RECIEVE,I2C_SEND}I2Cstatus;
struct i2cStatus
{
int bufferLength;
unsigned char Buffer[128];
unsigned char OutBuffer[128];
int outBufferLength;
int outBuffIndex;
I2Cstatus status;

}icStatus;
char i2c_data;
int data_read;
int data_written;
int sendMessage;

int CheckClock()
{
    SSPCON1bits.CKP;
}

int CheckBuffer()
{
    return SSPSTATbits.BF;
}
int CheckStart()
{
    return SSPSTATbits.P;
}
int CheckDataNOTAddress()
{
    return SSPSTATbits.D_A;
}
int CheckReadWrite()
{
    SSPSTATbits.R_W;
}

//I2C handler

void I2CR_int_handler()
{

    if(CheckBuffer())
    {
        data_read = 1;
    }
    switch(icStatus.status)
    {
        case I2C_IDLE:
        {
            if(CheckStart())
            {
                if(!CheckDataNOTAddress())
                {

                }
            }
            else if(data_read)
            {
                if(!CheckDataNOTAddress())
                {
                    if(!CheckReadWrite())
                    {
                        icStatus.status = I2C_RECIEVE;
                    }
                    else
                    {
                        icStatus.status = I2C_SEND;
                        data_read = 0;
                    }
                }
            }
        }
        case I2C_SEND:
        {
            if(icStatus.outBufferLength > icStatus.outBuffIndex)
            {
                SSPBUF = icStatus.OutBuffer[icStatus.outBuffIndex];
                icStatus.outBuffIndex++;
                data_written = 1;

            }
            else if(data_written == 0)
            {

              SSPBUF = 0xFF;
              data_written = 1;

            }
            else
            {
                
                data_written = 0;
                icStatus.outBuffIndex = 0;
                icStatus.outBufferLength =0;
                icStatus.status = I2C_IDLE;
            }
        }
        case I2C_RECIEVE:
        {
        }
    }
    if(data_read || data_written)
    {
        if(!CheckClock())
        {
            SSP1CON1bits.CKP = 1;
        }
    }

}
void InitI2C()
{
 //SSP1STATbits
    icStatus.OutBuffer[0] = 0x7E;
    icStatus.bufferLength = 0;
    icStatus.outBuffIndex = 0;
    icStatus.outBufferLength = 0;
    icStatus.status = I2C_IDLE;
    sendMessage = 0;
    data_read = 0;

#if TEST == 1

SSP1ADD = 0x9E;
OpenI2C1(SLAVE_7,SLEW_OFF);
// SSPSTAT = 0;
// SSP1CON2 = 0x0;
// SSP1CON1 = 0;
// SSP1CON1 = 0x0E;
// //SSP1CON1 = SLAVE_7;
//
 TRISCbits.TRISC3 = 1;
 TRISCbits.TRISC4 = 1;
 I2C1_SDA = 1;
 I2C1_SCL = 1;
#endif
#if TEST == 2
         // ensure the two lines are set for input (we are a slave)
        TRISCbits.TRISC3=1;
        TRISCbits.TRISC4=1;
        // set the address
        SSPADD = 0x9E;
        //OpenI2C(SLAVE_7,SLEW_OFF); // replaced w/ code below
        SSPSTAT = 0x0;
        SSPCON1 = 0x0;
        SSPCON2 = 0x0;
        SSPCON1 |= 0x0E;  // enable Slave 7-bit w/ start/stop interrupts
        SSPSTAT |= SLEW_OFF;
        I2C1_SDA = 1;
        I2C1_SCL = 1;
        // enable clock-stretching
        SSPCON2bits.SEN = 1;
        SSPCON1 |= SSPENB;

        // end of i2c configure
#endif


// SSPSTAT |= SLEW_OFF;
// I2C1_SCL = 1;
// I2C1_SDA = 1;
// SSPCON2bits.SEN = 1;
 //SSPCON1 |= SSPENB;
 //SSP1CON2bits
}


