#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <uart_init.h>

const long Baud_table[][2] = {
	{9600, B9600},
	{19200,B19200},
	{38400, B38400},
	{57600, B57600},
	{115200, B115200}
	};

int uart_nonblock_init(int* serial,char *file,speed_t* speed, struct termios* option_old, long *timeout)
{
	int errno_saved;
	struct termios option;
	speed_t baud;

	*serial = open(file,O_RDWR | O_NOCTTY | O_NONBLOCK);
	if(*serial == -1){
		errno_saved = errno;
		printf(" fail to open serial\n error: %d\n",errno_saved);
		return -1;
	}

	int ret_val = tcgetattr( *serial, option_old);
	if( ret_val != 0){
		errno_saved = errno;
		printf("error copying old termios setting\n Errno: %d\n", errno_saved);
		exit(EXIT_FAILURE);
	}

	ret_val = tcgetattr(*serial, &option);
	if( ret_val != 0){
		errno_saved = errno;
		printf("error copying\n Errno: %d\n", errno_saved);
		exit(EXIT_FAILURE);
	}


	option.c_cflag |= ( CLOCAL | CREAD);	// local connection, ignore modern detection
												// enable input to be read from terminal
	option.c_cflag |= ( HUPCL );		// Generate a modem disconnect signal -. drop DTR

	option.c_cflag &= ~CSTOPB;		// clear this bit -. one stop bit
	option.c_cflag &= ~PARENB;		// clear parity enable;
	option.c_cflag &= ~CSIZE;
	option.c_cflag |= (CS8);			// 8 bits per character
	option.c_cflag &= ~CRTSCTS;

	option.c_iflag &= ~INPCK;			// disable input parity check
	option.c_iflag &= ~ISTRIP;		// valid byte is not striped to 7-bits
	option.c_iflag &= ~IGNCR;			// input '\r' is discard
 	option.c_iflag &= ~INLCR;			// input '\n' would not be convert to '\r'
	option.c_iflag |= ~ICRNL;			// if IGNCR is clear, then \r -. \n	

	option.c_oflag &= ~OPOST;
	option.c_oflag &= ~(IXON | IXOFF | IXANY);

	option.c_cc[VMIN] = 0;			// would not block if no byte available
	option.c_cc[VTIME]= *timeout;			// read timeout is 10 * 100ms = 1 Second

	option.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
										// ISIG clear: INTR, QUIT, STOP characters not recognized
										
	int index;
	for( index = 0; index < (sizeof(Baud_table)/sizeof(Baud_table[0]));index++)
	{
		if( Baud_table[index][0] == *speed){
			baud = Baud_table[index][1];
			break;
		}
	}
	if( index == (sizeof(Baud_table)/sizeof(Baud_table[0]))){
		printf(" Baud rate %d is not supported by this program\n",*speed);
		return -1;
	}

	ret_val = cfsetispeed(&option, baud);
	if( ret_val == -1){
		errno_saved = errno;
		printf( "error setting input baudi\nerror number %d\n", errno_saved);
		return -1;
	}

	ret_val = cfsetospeed(&option, baud);
	if( ret_val == -1){
		errno_saved = errno;
		printf( "error setting input baudi\nerror number %d\n", errno_saved);
		return -1;
	}

	tcflush( *serial, TCIFLUSH);				// flush data recived but not read

	if ( tcsetattr( *serial, TCSANOW, &option) !=0){
		perror("setting stdin term error\nrestore previous terminal setting\n");
		if( tcsetattr( *serial, TCSANOW, option_old)!= 0)
			perror( " restore previous tty setting fail");
		return (-1);
	}	
}
