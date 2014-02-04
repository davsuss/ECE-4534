/* 
 * File:   I2C.h
 * Author: David
 *
 * Created on January 28, 2014, 5:58 PM
 */
#include "messages.h"
#include "i2c_thread.h"
#ifndef I2C_H
#define	I2C_H


void init_I2C_thread(i2c_thread_struct *);
void I2C_thread(i2c_thread_struct*,int,int,unsigned char*);
void InitI2C();

#endif	/* I2C_H */

