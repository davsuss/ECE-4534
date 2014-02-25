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
char lastMotorCommand = 0xFF;
#endif	/* MOTOR_THREAD_H */

