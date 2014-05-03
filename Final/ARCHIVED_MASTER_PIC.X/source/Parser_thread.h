/*
 * File:   Parser_thread.h
 * Author: David, Arjun Passi
 *
 * Created on February 25, 2014, 1:09 AM
 */

#ifndef PARSER_THREAD_H
#define	PARSER_THREAD_H
unsigned int FOOT = 0x58;
unsigned int INCH = 0x03;

unsigned int TENDEGREES = 0x0A;
unsigned int NINETYDEGREES = 0x60;
unsigned int Started = 0;
int parser_lthread(int,int,unsigned char*);

/*
 * Public API call to find out whether the message
 * received from the ARM is valid or not. This API
 * call must be called everytime a new message is recieved
 * in order to use other API calls defined.
 *
 * Args:
 *  unsigned char buffer - Buffer containing the message from the rover
 *  unsigned char length - Length of the passed buffer
 *
 * Return:
 * unsigned char - 1 if message is valid otherwise 0
 */
unsigned char validateMessage(unsigned char buffer [], unsigned char length);

/*
 * Public function call to retrieve the message id
 * present in the message from the ARM.
 *
 * Args:
 *  unsigned char buffer - Buffer containing the message from the rover
 *  unsigned char length - Length of the passed buffer
 *
 * Return:
 *  unsigned char - message id present in the buffer
 */
unsigned char getMessageId(unsigned char buffer [], unsigned char length);

#endif	/* PARSER_THREAD_H */