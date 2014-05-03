#include "messages.h"
#include "maindefs.h"
#include "my_i2c.h"
#include "ArmMessageTypes.h"
#include "Parser_thread.h"
#include "my_uart.h"
unsigned int motorStatus;
int parser_lthread(int msgtype,int length,unsigned char* msgbuf)
{
    if(msgtype == MSGT_PARSE) {

        if ( !validateMessage(msgbuf, (unsigned char) length) )
            return;

        switch (getMessageId(msgbuf, (unsigned char) length)) {

            case SENSOR_REQUEST:
            {
                i2c_configure_master(0x4D);
                i2c_master_recv(0x07);
                break;
            };

            case COMMAND:
            {
                unsigned char data[5];
                unsigned char command;
                unsigned int ticks;
                unsigned char Feet = 0;
                unsigned char Inches = 0;
                unsigned char degrees = 0;

                switch(msgbuf[1] & 0x0F)
                {
                    case(0x01):
                    {
                        //left
                        command = 0x01;
                        degrees = msgbuf[2];
                        break;
                    }
                    case(0x02):
                    {
                        //right
                        command = 0x02;
                        degrees = msgbuf[2];
                        break;
                    }
                    case(0x00):
                    {
                        //Straight
                       command = 0x03;
                       Feet = msgbuf[3];
                       Inches = msgbuf[4];
                       
                       break;
                    }
                }

                data[0] = command;
                data[1] = degrees;
                data[2] = Feet;
                data[3] = Inches;

                unsigned char reply[6];
                reply[0] = 0x30;
                reply[1] = 0x00;
                reply[2] = 0x00;
                reply[3] = 0x00;
                reply[4] = 0x00;
                reply[5] = 0x30;

                uart_write(6,reply);

                i2c_configure_master(0x4F);
                i2c_master_send(5,data);

                break;
            };

            case START:
            {
                unsigned char reply[6];
                reply[0] = 0x30;
                reply[1] = 0x00;
                reply[2] = 0x00;
                reply[3] = 0x00;
                reply[4] = 0x00;
                reply[5] = 0x30;

                uart_write(6,reply);
                break;
            };

            case STOP:
            {
                unsigned char reply[6];
                reply[0] = 0x30;
                reply[1] = 0x00;
                reply[2] = 0x00;
                reply[3] = 0x00;
                reply[4] = 0x00;
                reply[5] = 0x30;

                uart_write(6,reply);
                break;
            };

            case MSG_DROP:
            {
                break;
            };

            default:
                break;
        }

    }
    if(msgtype == MSGT_SEND_BACK)
    {
        unsigned int checksum = 0;
        unsigned char data[12];
        unsigned char status = 0;

        if(msgbuf[6] == 0x01)
        {
            status = 0x20;
        }
        data[0] = status | 0x06;
        data[1] = msgbuf[0];
        data[2] = msgbuf[1];
        data[3] = msgbuf[2];
        data[4] = msgbuf[3];
        data[5] = msgbuf[4];
        data[6] = msgbuf[5];
        data[7] = 0x00;
        data[8] = 0x00;
        data[9] = 0x00;
        data[10] = 0x00;
        int x = 0;
        for(x = 0; x < 11;x++)
        {
            checksum += data[x];
        }
        data[11] = checksum & 0x0FF;
        uart_write(12,data);
    }

  

}

/*
 * Public API call to find out whether the message
 * received from the ARM is valid or not. This API
 * call must be called everytime a new message is recieved
 * in order to use other API calls defined.
 *
 * Args:
 *  unsigned char buffer - Buffer containing the message from the rover
 *  unsigned char length - Length of the passed buffer
 *
 * Return:
 * unsigned char - 1 if message is valid otherwise 0
 */
unsigned char validateMessage(unsigned char buffer [], unsigned char length)
{
    // Check Sum and the MSG ID must
    // be present in the buffer

    if (length < 2) {
        return 0;
    }

    unsigned char len = buffer[0] & 0xF;
    unsigned char buf = buffer[5];
    unsigned char buf2 = buffer[4];
    unsigned char buf3 = buffer[3];
    // Multiplying length by 4
    // CMD Type, Turn Deg, Dist Feet, Dist Inches
    len = len;

    // MSG ID + Len + Check Sum
    len = 1 + len + 1;
    int i = 0;
    unsigned char checksum= buffer[len - 1];
    unsigned char rollingsum = 0x00;

    for(i = 0; i < len - 1; i++)
    {
        rollingsum += buffer[i];
    }

    return ( (rollingsum & 0xFF) == checksum ) ? 1 : 0;
}

/*
 * Public function call to retrieve the message id
 * present in the message from the ARM.
 *
 * Args:
 *  unsigned char buffer - Buffer containing the message from the rover
 *  unsigned char length - Length of the passed buffer
 *
 * Return:
 *  unsigned char - message id present in the buffer
 */
unsigned char getMessageId(unsigned char buffer [], unsigned char length)
{
    if (length > 0) {
        return (buffer[0] & 0xF0) >> 4;
    }

    else return MSG_DROP;
}