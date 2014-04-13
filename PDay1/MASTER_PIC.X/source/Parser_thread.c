int

#include "messages.h"
#include "maindefs.h"
#include "my_i2c.h"
 parser_lthread(int msgtype,int length,unsigned char* msgbuf)
{
    if(msgtype == MSGT_PARSE)
    {
    length /= sizeof(unsigned char);
    int i = 0;
    char checksum = 0x0;
    
    for(i = 0; i < length-1; i++)
    {
        checksum += msgbuf[i];
    }
    if(checksum == msgbuf[length-1])
    {
       
        char data[5];
        char Command = (msgbuf[1] >> 4) & 0xF;
     //Valid Message
        if(Command == 0x02)
        {
            char movement = msgbuf[1] & 0x0F;

            if(movement == 0x0)
            {
                data[0] = 0x03;
            }
            else if(movement == 0x01)
            {
                data[0] = 0x02;
            }
            else if(movement == 0x02)
            {
                data[0] = 0x01;
            }
            else
            {
                data[0] = 0x04;
            }

            data[1] = msgbuf[2];
            data[2] = msgbuf[3];
            data[3] = msgbuf[2];
            data[4] = msgbuf[3];

            i2c_configure_master(0x4F);
            i2c_master_send(0x05,&data);            
        }
        else if(Command == 0x00)
        {

            //i2c_configure_master(0x4F);
            //i2c_master_recv(5);
        }



    }
    else if(msgtype == MSGT_SEND_BACK )
    {
        
    }




    }




}
