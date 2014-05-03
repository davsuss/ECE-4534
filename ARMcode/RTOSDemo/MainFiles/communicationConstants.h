#ifndef COMMUNICATION_CONSTANTS_H
#define COMMUNICATION_CONSTANTS_H

//----------------------------------------
//possible command ids
#define start_cmd_ToRover 0
#define stop_cmd_ToRover 1
#define slow_cmd_ToRover 2
#define fast_cmd_ToRover 3
//----------------------------------------
//possible turn ids
#define straight_turn_ToRover 0 
#define left_turn_ToRover 1
#define right_turn_ToRover 2
//----------------------------------------
//possible msgIDs
#define sensorReq_msgID_ToRover 0
#define sesorReplyPoll_msgID_ToARMPic 1
#define sendCommand_msgID_ToRover 2
#define sendStop_msgID_ToRover 3
#define sendStart_msgID_ToRover 4
//-----------------------------------------


#endif
