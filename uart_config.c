/****************************************************/
//
//			Author: Da Ke
//			Date:	04/24/2015
// 
// 
// 
/*****************************************************/
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <fcntl.h>


#include "uart_config.h"

/**	 	Set terminal to RAW mode only. 
 * 		For transparent serial port: use Set_Serial_Port() funtion,
 * **/
void Set_RAW_Mode( struct termios* termios_p)
{
	// Ideal for a transparent serial I/O port
	
	/**		input mode config					**/		
			termios_p->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP \
                           		 | INLCR | IGNCR | ICRNL | IXON);
						
	/**		Disable all post output processing	**/
           	termios_p->c_oflag &= ~OPOST;
	
	/**		Mode & entention disable 			**/
           	termios_p->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);

	/**		Disable input parity check & output parity geration 	**/
           	termios_p->c_cflag &= ~(CSIZE | PARENB);
           	termios_p->c_cflag |= CS8;
}


// function return linespeed in POSIX compatible format
speed_t Set_Baud(long* baud)
{
	speed_t linespeed;
	/*****   For POSIX portable, baud rate format is converted *********/	
	int baudMapping[][2]={
					{9600, B9600},
					{ 57600, B57600},
					{ 115200, B115200}
		};
	int MappingNumber = sizeof(baudMapping)/sizeof(baudMapping[0]);

	int j;
	for( j = 0; j < MappingNumber; j++)
	{
		if( baudMapping[j][0] == *baud)
		{
			linespeed = baudMapping[j][1];
			break;
		}
	}
	return linespeed;
}

int Set_Serial_Port(int fd, char* filepath,struct termios* option_4_restore,long* baud, int timeout)
{
	speed_t linerate;
	struct termios option_4_set;
	struct termios* termios_p;

	termios_p = &option_4_set;

	fd = open(filepath,O_NOCTTY | O_RDWR | O_NONBLOCK );
	
	if( tcgetattr(fd, option_4_restore) != 0){
		perror("get old termios setting");
		return -1;
	}

	if( tcgetattr(fd, termios_p) != 0){
		perror("get termios for modification");
		return -1;
	}
	
	Set_RAW_Mode(termios_p);
	linerate = Set_Baud(baud);

	/**	 8-N-1, no hard flow control 	**/ 
	termios_p->c_cflag &= ~( CSTOPB | CRTSCTS );
	/**  Enable input receive 			**/
	termios_p->c_cflag |= ( CREAD | CLOCAL | HUPCL );                             
	                                                                             
/** ----------+--------------+--------------------+-------------------------------+
 * 			  | No Bytes 	 |   Requested 		  | 	Requested                 |
|* 			  | 			 |  	>			  | 		<                     |
|   		  | 			 |     FIFO			  | 	   FIFO                   |
+-------------+--------------+--------------------+-------------------------------+
|MIN = 0      | immediately	 |  				  |                               |
|TIME = 0 	  | return 0	 |  return all		  | 	return requested          |
|   		  ------------------------------------+-------------------------+-----|
|   		  | *Note: if O_NONBLOCK is set, then No byte read would return       |
|   		  | 		-1 and with the error EAGAIN                              |
+-------------+--------------+--------------------+-------------------------------+
|MIN > 0      |              |                    |                               |
|TIME = 0	  | Block wait	 |  return FIFO		  | 	return requested          |
|(Block Read) |              |                    |                               |
+-------------+--------------+--------------------+-------------------------------+
|MIN =0       |                                                                   |
|TIME >0	  | Block or time                                                     |
|read w timeou| out and return                                                    |
|   		  | 0                                                                 |
|   		  +-------------------------------------------------------------------|
|   		  | *Note: timeout is TIME tenths of a second; useful for serial      |
|   		  | 		device                                                    |
|-------------+--------------+---------------------+------------------------------+
|MIN > 0      |              |                     |                              |
|TIME > 0 	  | BLOCK        |                     |                              |
|             +--------------+---------------------+------------------------------+
|   		  | *Note: after initial byte, timer is restarted as each further     |
|   		  | 		byte is received; Useful for terminal keys that generate  |
|   		  | 		escape sequence                                           |
+-------------+-------------------------------------------------------------------+
 */
	termios_p->c_cc[VMIN] = 0;
	termios_p->c_cc[VTIME] = 100;						// (TIME/10) second timeout

	cfsetispeed(termios_p,linerate);
	if( linerate != cfgetispeed(termios_p)) return -1;
	cfsetospeed(termios_p,linerate);
	if( linerate != cfgetospeed(termios_p)) return -1;
	
	if(tcsetattr( fd, TCSANOW, termios_p ) != 0)
		return -1;

	return 0;
	
}

int main()
{
	int i;
	while(i)
	{
		i++;
		if(i > 7000)
			i = 0;
	}
	return 0;
}
