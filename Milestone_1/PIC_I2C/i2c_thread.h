/* 
 * File:   i2c_thread.h
 * Author: David
 *
 * Created on January 30, 2014, 11:08 AM
 */

#ifndef I2C_THREAD_H
#define	I2C_THREAD_H

typedef struct __i2c_thread_struct {
	int	data;
} i2c_thread_struct;

void init_i2c_lthread(i2c_thread_struct *);
int i2c_lthread(i2c_thread_struct *,int,int,unsigned char*);

#endif	/* I2C_THREAD_H */

