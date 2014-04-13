/* 
 * File:   Motor_thread.h
 * Author: David
 *
 * Created on February 23, 2014, 4:01 PM
 */

#ifndef MOTOR_THREAD_H
#define	MOTOR_THREAD_H
//int timer0_lthread(timer0_thread_struct *,int,int,unsigned char*);
int motor_lthread(int,int,unsigned char*);
unsigned int motorDistance = 0x00;
int OverflowsR = 0;
int OverflowsL = 0;
int ActiveR = 0;
int ActiveL = 0;
int NumOfOverflowR = 0;
int NumOfOverflowL = 0;

unsigned int DeltaOverflowR = 0;
unsigned int DeltaOverflowL = 0;

#endif	/* MOTOR_THREAD_H */

