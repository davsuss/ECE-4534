int

#include "messages.h"
#include "maindefs.h"
#include "my_i2c.h"
 parser_lthread(int msgtype,int length,unsigned char* msgbuf)
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
        char Command = (msgbuf[1] >> 4) & 0xF;
     //Valid Message
        if(Command == 0x02)
        {
            char data[3];
            data[0] = 0x07;
            data[1] = 0x01;
            data[2] = 0x02;
            i2c_configure_master(0x4F);
            i2c_master_send(0x03,&data);
            //ToMainHigh_sendmsg(sizeof(data),MSGT_UART_SEND,(void*)&data);

            
        }
        else if(Command == 0x00)
        {

            unsigned char data[9];
            data[0] = 0;
            data[1] = 0x02;
            data[2] = 0x00;
            data[3] = 0x00;
            data[4] = 0x01;
            data[5] = 0x02;
            data[6] = 0x03;
            data[7] = 0x04;
            data[8] = 0x0C;
            ToMainHigh_sendmsg(sizeof(data),MSGT_UART_SEND,(void*)&data);
        }








    }



}
