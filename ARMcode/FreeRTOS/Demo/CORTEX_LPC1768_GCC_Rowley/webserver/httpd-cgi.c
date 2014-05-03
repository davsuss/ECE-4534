/**
 * \addtogroup httpd
 * @{
 */

/**
 * \file
 *         Web server script interface
 * \author
 *         Adam Dunkels <adam@sics.se>
 *
 */

/*
 * Copyright (c) 2001-2006, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: httpd-cgi.c,v 1.2 2006/06/11 21:46:37 adam Exp $
 *
 */

#include "uip.h"
#include "psock.h"
#include "httpd.h"
#include "httpd-cgi.h"
#include "httpd-fs.h"
#include "webservertask.h"
#include "webServerMessageTypes.h"

#include <stdio.h>
#include <string.h>

HTTPD_CGI_CALL(file, "file-stats", file_stats);
HTTPD_CGI_CALL(tcp, "tcp-connections", tcp_stats);
HTTPD_CGI_CALL(net, "net-stats", net_stats);
HTTPD_CGI_CALL(rtos, "rtos-stats", rtos_stats );
HTTPD_CGI_CALL(run, "run-time", run_time );
HTTPD_CGI_CALL(io, "led-io", led_io );
HTTPD_CGI_CALL(map, "points", map_run);
HTTPD_CGI_CALL(sensorandcommands, "sensorcmd", sensor_command_run);


static const struct httpd_cgi_call *calls[] = { &file, &tcp, &net, &rtos, &run, &io, &map, &sensorandcommands, NULL };

/*---------------------------------------------------------------------------*/
static
PT_THREAD(nullfunction(struct httpd_state *s, char *ptr))
{
    PSOCK_BEGIN(&s->sout);
    ( void ) ptr;
    PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
httpd_cgifunction
httpd_cgi(char *name)
{
    
    const struct httpd_cgi_call **f;
    
    /* Find the matching name in the table, return the function. */
    for(f = calls; *f != NULL; ++f) {
        if(strncmp((*f)->name, name, strlen((*f)->name)) == 0) {
            return (*f)->function;
        }
    }
    return nullfunction;
}
/*---------------------------------------------------------------------------*/
static unsigned short
generate_file_stats(void *arg)
{
    char *f = (char *)arg;
    return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE, "%5u", httpd_fs_count(f));
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(file_stats(struct httpd_state *s, char *ptr))
{   
    PSOCK_BEGIN(&s->sout);
    
    PSOCK_GENERATOR_SEND(&s->sout, generate_file_stats, strchr(ptr, ' ') + 1);
    
    PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static const char closed[] =   /*  "CLOSED",*/
{0x43, 0x4c, 0x4f, 0x53, 0x45, 0x44, 0};
static const char syn_rcvd[] = /*  "SYN-RCVD",*/
{0x53, 0x59, 0x4e, 0x2d, 0x52, 0x43, 0x56,
    0x44,  0};
static const char syn_sent[] = /*  "SYN-SENT",*/
{0x53, 0x59, 0x4e, 0x2d, 0x53, 0x45, 0x4e,
    0x54,  0};
static const char established[] = /*  "ESTABLISHED",*/
{0x45, 0x53, 0x54, 0x41, 0x42, 0x4c, 0x49, 0x53, 0x48,
    0x45, 0x44, 0};
static const char fin_wait_1[] = /*  "FIN-WAIT-1",*/
{0x46, 0x49, 0x4e, 0x2d, 0x57, 0x41, 0x49,
    0x54, 0x2d, 0x31, 0};
static const char fin_wait_2[] = /*  "FIN-WAIT-2",*/
{0x46, 0x49, 0x4e, 0x2d, 0x57, 0x41, 0x49,
    0x54, 0x2d, 0x32, 0};
static const char closing[] = /*  "CLOSING",*/
{0x43, 0x4c, 0x4f, 0x53, 0x49,
    0x4e, 0x47, 0};
static const char time_wait[] = /*  "TIME-WAIT,"*/
{0x54, 0x49, 0x4d, 0x45, 0x2d, 0x57, 0x41,
    0x49, 0x54, 0};
static const char last_ack[] = /*  "LAST-ACK"*/
{0x4c, 0x41, 0x53, 0x54, 0x2d, 0x41, 0x43,
    0x4b, 0};

static const char *states[] = {
    closed,
    syn_rcvd,
    syn_sent,
    established,
    fin_wait_1,
    fin_wait_2,
    closing,
    time_wait,
    last_ack};


static unsigned short
generate_tcp_stats(void *arg)
{   
    struct uip_conn *conn;
    struct httpd_state *s = (struct httpd_state *)arg;
    
    conn = &uip_conns[s->count];
    return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
                    "<tr><td>%d</td><td>%u.%u.%u.%u:%u</td><td>%s</td><td>%u</td><td>%u</td><td>%c %c</td></tr>\r\n",
                    htons(conn->lport),
                    htons(conn->ripaddr[0]) >> 8,
                    htons(conn->ripaddr[0]) & 0xff,
                    htons(conn->ripaddr[1]) >> 8,
                    htons(conn->ripaddr[1]) & 0xff,
                    htons(conn->rport),
                    states[conn->tcpstateflags & UIP_TS_MASK],
                    conn->nrtx,
                    conn->timer,
                    (uip_outstanding(conn))? '*':' ',
                    (uip_stopped(conn))? '!':' ');
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(tcp_stats(struct httpd_state *s, char *ptr))
{   
    PSOCK_BEGIN(&s->sout);
    ( void ) ptr;
    for(s->count = 0; s->count < UIP_CONNS; ++s->count) {
        if((uip_conns[s->count].tcpstateflags & UIP_TS_MASK) != UIP_CLOSED) {
            PSOCK_GENERATOR_SEND(&s->sout, generate_tcp_stats, s);
        }
    }
    
    PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static unsigned short
generate_net_stats(void *arg)
{   
    struct httpd_state *s = (struct httpd_state *)arg;
    return snprintf((char *)uip_appdata, UIP_APPDATA_SIZE,
                    "%5u\n", ((uip_stats_t *)&uip_stat)[s->count]);
}

static
PT_THREAD(net_stats(struct httpd_state *s, char *ptr))
{   
    PSOCK_BEGIN(&s->sout);
    
    ( void ) ptr;
#if UIP_STATISTICS
    
    for(s->count = 0; s->count < sizeof(uip_stat) / sizeof(uip_stats_t);
        ++s->count) {
        PSOCK_GENERATOR_SEND(&s->sout, generate_net_stats, s);
    }
    
#endif /* UIP_STATISTICS */
    
    PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/

extern void vTaskList( signed char *pcWriteBuffer );
extern char *pcGetTaskStatusMessage( void );
static char cCountBuf[ 256 ];
uint16_t width = 300;
uint16_t height = 300;
char buf[256];
 
long lRefreshCount = 0;
static unsigned short
generate_rtos_stats(void *arg)
{   
	( void ) arg;
	lRefreshCount++;
	sprintf( cCountBuf, "<p><br>Refresh count = %d<p><br>%s", (int)lRefreshCount, pcGetTaskStatusMessage() );
    vTaskList( uip_appdata );
	strcat( uip_appdata, cCountBuf );
	return strlen( uip_appdata );
}
/*---------------------------------------------------------------------------*/


static
PT_THREAD(rtos_stats(struct httpd_state *s, char *ptr))
{   
    PSOCK_BEGIN(&s->sout);
    ( void ) ptr;
    PSOCK_GENERATOR_SEND(&s->sout, generate_rtos_stats, NULL);
    PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/

char *pcStatus;
unsigned long ulString;

static unsigned short generate_io_state( void *arg )
{
    extern long lParTestGetLEDState( void );
    
	( void ) arg;
    
	/* Get the state of the LEDs that are on the FIO1 port. */
	if( lParTestGetLEDState() )
	{
		pcStatus = "";
	}
	else
	{
		pcStatus = "checked";
	}
    
	sprintf( uip_appdata,
            "<input type=\"checkbox\" name=\"LED0\" value=\"1\" %s>LED<p><p>", pcStatus );
    
	return strlen( uip_appdata );
}
/*---------------------------------------------------------------------------*/

extern void vTaskGetRunTimeStats( signed char *pcWriteBuffer );
static unsigned short
generate_runtime_stats(void *arg)
{   
	( void ) arg;
	lRefreshCount++;
	sprintf( cCountBuf, "<p><br>Refresh count Arjun= %d", (int)lRefreshCount );
    vTaskGetRunTimeStats( uip_appdata );
	strcat( uip_appdata, cCountBuf );
	return strlen( uip_appdata );
}

/*---------------------------------------------------------------------------*/

static
PT_THREAD(run_time(struct httpd_state *s, char *ptr))
{
    
    PSOCK_BEGIN(&s->sout);
    ( void ) ptr;
    PSOCK_GENERATOR_SEND(&s->sout, generate_runtime_stats, NULL);
    PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/

extern void vTaskGetMapStats( signed char *pcWriteBuffer );
static unsigned short
generate_map_stats(void *arg)
{
 	(void) arg;
	lRefreshCount++;

	uint16_t distance = getDistance();
	uint16_t time = getTime();

	if( getMaxXcoord() < ((width/20) - 1))
	 	width = 300;

	if( getMaxYcoord() < ((height/20) - 1))
	 	height = 300;
		

	while( getMaxXcoord() > ((width/20) - 1))
		width += 100;

	while( getMaxYcoord() > ((height/20) - 1))
		height += 100;


	//<sensors hidden id="sensors">Hello Sensors</sensors>
	//		<commands hidden id="commands">Hello Commands</commands>



	//sprintf( cCountBuf,
	// "<div class=\"alert alert-success\"><p>Distance Travelled: %d</p><p>Time Elapsed: %d</p></div><br>",distance, time);



//	vTaskGetMapStats( uip_appdata );

	sprintf( buf, "<dist_ hidden id=\"dist_\">%d</dist_>", 5);
	strcat( uip_appdata, buf);

	sprintf( buf, "<time_ hidden id=\"time_\">%d</time_>", 5);
	strcat( uip_appdata, buf);

	sprintf(cCountBuf, "<width hidden id=\"width\">%d</width><br>", width);
	strcat( uip_appdata, cCountBuf);


	sprintf(cCountBuf, "<height hidden id=\"height\">%d</height><br>", height);
	strcat( uip_appdata, cCountBuf);

	sprintf(buf,"<x hidden id='x'>");
	strcat( uip_appdata, buf);
    
    
    int i = 0;
    for (i = 0; i < getNumCoordinates(); i++) {
        sprintf(buf, "%d ", getXCoordinate(i));
        strcat( uip_appdata, buf);
    }

	sprintf(buf,"</x>\n");
	strcat( uip_appdata, buf);

	sprintf(buf,"<y hidden id='y'>");
	strcat( uip_appdata, buf);

	for (i = 0; i < getNumCoordinates(); i++) {
        sprintf(buf, "%d ", getYCoordinate(i));
        strcat( uip_appdata, buf);
    } 
    
	sprintf(buf,"</y>\n");
	strcat( uip_appdata, buf);


	return strlen( uip_appdata );
    
}

/*---------------------------------------------------------------------------*/
//Updating the char *ptr will change the map.shtml file
static
PT_THREAD(map_run(struct httpd_state *s, char *ptr))
{   
	PSOCK_BEGIN(&s->sout);
	( void ) ptr;
	PSOCK_GENERATOR_SEND(&s->sout, generate_map_stats, NULL);
	PSOCK_END(&s->sout);
}

static unsigned short
generate_sensor_commands(void *arg)
{
   	
	uint16_t distance = getDistance();
	uint16_t time = getTime();

	sprintf( buf, "<dist_ hidden id=\"dist_\">%d</dist_>", distance);
	strcat( uip_appdata, buf);

	sprintf( buf, "<time_ hidden id=\"time_\">%d</time_>", time);
	strcat( uip_appdata, buf);


	sprintf(cCountBuf,"<sensors hidden id=\"sensors\">");
	strcat(uip_appdata, cCountBuf);

	if (sensorValueBufferRead() == 0)
	{
	 	strcat(uip_appdata, getSensorValueString());
		setSensorValueFlag(1);
	}

	strcat(uip_appdata, "</sensors>");
	strcat(uip_appdata,"<commands hidden id=\"commands\">");

	if (commandIssuedBufferRead() == 0)
	{
	 	strcat(uip_appdata, getCommandIssuedString());
		setCommandIssueFlag(1);
	}

	strcat(uip_appdata, "</commands>");
	
	//cCountBuf for sensors
	//use buf for commands

//	sprintf(cCountBuf,"<sensors hidden id=\"sensors\">");
//	sprintf(buf,"<commands hidden id=\"commands\">");
//
//	//Web Server Message
//    webServerMSG wsMsg;
//	if ( WebServerDeQ(&wsMsg, portMAX_DELAY)	== pdTRUE )
//	{
//		switch ( wsMsg.msgType ) {
//
//			case sensorData:
//			{
//				uint8_t frontDistance = wsMsg.sensorDistancesBuf[FRONT_SENSOR_INDEX];
//   				uint8_t topRightDistance = wsMsg.sensorDistancesBuf[TOP_RIGHT_SENSOR_INDEX] = topRightDistance;
//    			uint8_t bottomRightDistance = wsMsg.sensorDistancesBuf[BOTTOM_RIGHT_SENSOR_INDEX] = bottomRightDistance;
//
//				sprintf(tempbuf, "Front Distance: %d , Top Right Distance: %d , Bottom Right Distance: %d",
//            			frontDistance, topRightDistance, bottomRightDistance);
//				strcat(cCountBuf, tempbuf);
//
//				printf("sensor data \n");
//				break;
//			};
//			
//			case commandIssued:
//			{
//
//				uint8_t commandType = wsMsg.commandsIssuedBuf[CMD_TYPE_INDEX];
//    			uint8_t turnType = wsMsg.commandsIssuedBuf[TURN_TYPE_INDEX];
//    			uint8_t turnDegrees = wsMsg.commandsIssuedBuf[TURN_DEGREES_INDEX];
//    			uint8_t distanceFeet = wsMsg.commandsIssuedBuf[DISTANCE_FEET_INDEX];
//    			uint8_t distanceInches = wsMsg.commandsIssuedBuf[DISTANCE_INCH_INDEX];
//
//				if (commandType == start)
//        			strcat(buf, "Command: START ,");
//    			else if (commandType == stop)
//        			strcat(buf, "Command: STOP ,");
//    			else if (commandType == slow)
//        			strcat(buf, "Command: SLOW ,");
//    			else
//        			strcat(buf, "Command: fast ,");
//    
//    			if (turnType == straight)
//        			strcat(buf, "Turn: STRAIGHT ,");
//    			else if (commandType == left)
//        			strcat(buf, "Turn: LEFT ,");
//    			else
//        			strcat(buf, "Turn: RIGHT ,");
//    
//    
//    			sprintf(tempbuf, "Turn Degrees: %d, Feet: %d , Inches: %d\n",
//            			turnDegrees, distanceFeet, distanceInches);
//				
//				strcat(buf, tempbuf);
//
//				printf("commands data \n");
//
//				break;
//			};			
//
//		}
//	}
//
//	strcat(cCountBuf, "</sensors>");
//	strcat(buf, "</commands>");
//
//	strcat(uip_appdata, cCountBuf);
//	strcat(uip_appdata, buf);
//
//	printf("uip_appdata %s\n", uip_appdata);

	return strlen( uip_appdata );
}

static
PT_THREAD(sensor_command_run(struct httpd_state *s, char *ptr))
{   
	PSOCK_BEGIN(&s->sout);
	( void ) ptr;
	PSOCK_GENERATOR_SEND(&s->sout, generate_sensor_commands, NULL);
	PSOCK_END(&s->sout);
}

/*---------------------------------------------------------------------------*/

static PT_THREAD(led_io(struct httpd_state *s, char *ptr))
{
    
    PSOCK_BEGIN(&s->sout);
    ( void ) ptr;																																								   
    PSOCK_GENERATOR_SEND(&s->sout, generate_io_state, NULL);
    PSOCK_END(&s->sout);
}

/** @} */






