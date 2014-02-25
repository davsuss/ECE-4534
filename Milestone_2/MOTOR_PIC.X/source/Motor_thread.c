#include "maindefs.h"
#include "messages.h"
#include "Motor_thread.h"
#define Fast_Forward_LM 0xAF
#define Slow_Forward_LM 0xB6
#define Fast_Backward_LM 0xDF
#define Slow_Backward_LM 0xC6
#define Stop 0x00
#define Fast_Forward_RM 0x1F
#define Slow_Forward_RM 0x36
#define Fast_Backward_RM 0x5F
#define Slow_Backward_RM 0x46
int motor_lthread(int msgtype,int length,unsigned char* msgbuffer)
{

    switch (msgtype)
    {
        case (MSGT_MOTOR_COMMAND):
        {
            char message[2];
            lastMotorCommand = msgbuffer[0];
            switch(msgbuffer[0])
            {
                case (0x01):
                {
                    //Turn Left Fast
                    message[0] = Fast_Backward_LM;
                    message[1] = Fast_Forward_RM;
                    break;

                };
                case (0x02):
                {
                    //Turn Right Fast
                    message[0] = Fast_Backward_LM;
                    message[1] = Fast_Forward_RM;
                    break;
                };
                case (0x03):
                {
                    //Straight Fast
                    message[0] = Fast_Forward_LM;
                    message[1] = Fast_Forward_RM;
                    break;
                };
                case (0x04):
                {
                    //Emergrency Stop
                    message[0] = Stop;
                    message[1] = Stop;
                    break;
                };
                case (0x05):
                {
                    //Turn Left Slow
                    message[0] = Slow_Backward_LM;
                    message[1] = Slow_Forward_RM;
                    break;
                };
                case (0x06):
                {
                    //Turn Right Slow
                    message[0] = Slow_Backward_RM;
                    message[1] = Slow_Forward_LM;
                    break;
                };
                default:
                {
                    //Straight Slow
                    message[0] = Slow_Forward_RM;
                    message[1] = Slow_Forward_LM;
                    break;
                };
            }
            
            ToMainHigh_sendmsg(2,MSGT_UART_SEND,&message );
            break;
        };
        default:
        {
          break;  
        };
        
        
        
        
    }



}
