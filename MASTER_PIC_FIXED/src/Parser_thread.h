/* 
 * File:   Parser_thread.h
 * Author: David
 *
 * Created on April 15, 2014, 10:14 AM
 */
#include "maindefs.h"
#ifndef PARSER_THREAD_H
#define	PARSER_THREAD_H

#ifdef	__cplusplus
extern "C" {
#endif

void parser_lthread(int msgtype,int length,unsigned char* msgbuf);
//
void StartSensorRequest();
//
void StartMotorRequest();
//
void ProcessSensorRequest();
//
void ProcessMotorRequest();






#ifdef	__cplusplus
}
#endif

#endif	/* PARSER_THREAD_H */

