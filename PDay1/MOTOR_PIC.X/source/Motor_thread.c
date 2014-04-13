#include "maindefs.h"
#include "messages.h"
#include "Motor_thread.h"
#define Fast_Forward_LM 0xAF
#define Slow_Forward_LM 0xB6
#define Fast_Backward_LM 0xDF
#define Slow_Backward_LM 0xC6
#define Stop 0x00
#define StopLeft 0x64
#define StopRight 0x44
#define Fast_Forward_RM 0x1F
#define Slow_Forward_RM 0x36
#define Fast_Backward_RM 0x5F
#define Slow_Backward_RM 0x46

int motor_lthread(int msgtype,int length,unsigned char* msgbuffer)
{

    switch (msgtype)
    {
        case (MSGT_MOTOR_STOP):
        {
            unsigned char message[1];
            message[0] = Stop;
            ToMainHigh_sendmsg(sizeof(message),MSGT_UART_SEND,&message );
            ActiveL = 0;
            ActiveR = 0;
            break;
        }
        case (MSGT_MOTOR_STOP_LEFT):
        {
            unsigned char message[1];
            message[0] = StopLeft;
            ToMainHigh_sendmsg(sizeof(message),MSGT_UART_SEND,&message );
            break;
        }
        case (MSGT_MOTOR_STOP_RIGHT):
        {
            unsigned char message[1];
            message[0] = StopRight;
            ToMainHigh_sendmsg(sizeof(message),MSGT_UART_SEND,&message );
            break;
        }
        case (MSGT_MOTOR_POWER_REDUCE_LEFT):
        {
            
        }
        case (MSGT_MOTOR_POWER_REDUCE_RIGHT):
        {
            
        }
        case (MSGT_MOTOR_COMMAND):
        {
            unsigned char message[2];
            ActiveL = 1;
            ActiveR = 1;
            NumOfOverflowL = 0;
            NumOfOverflowR = 0;
            WriteTimer0(0);
            WriteTimer1(0);
            OverflowsL = ((msgbuffer[1] << 8) | (msgbuffer[2]));
            OverflowsR = ((msgbuffer[3] << 8) | (msgbuffer[4]));
            switch(msgbuffer[0])
            {
                case (0x01):
                {
                    //Turn Left Fast
                    message[0] = 0xE3;
                    message[1] = 0x1F;
                    break;

                };
                case (0x02):
                {
                    //Turn Right Fast
                    message[0] = 0x5C;
                    message[1] = 0xA0;
                    break;
                };
                case (0x03):
                {
                    //Straight Fast
                    message[0] = 0x1F;
                    message[1] = 0x9D;
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
                case (0x10):
                {
                    message[0] = Stop;
                    message[1] = 0xB6;
                    break;

                };
                case (0x11):
                {
                    message[0] = Stop;
                    message[1] = Slow_Forward_RM;
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
            
            ToMainHigh_sendmsg(sizeof(message),MSGT_UART_SEND,&message );
            break;
        };
        default:
        {
          break;  
        };
        
        
        
        
    }



}
