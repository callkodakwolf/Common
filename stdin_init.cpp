#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <stdlib.h>

#include "stdin_init.h"


int stdin_nonblock_init(int* tty_in,struct termios* option_old)
{
	struct termios option_ttyin;
	int ret_val, errno_saved;
	*tty_in = dup(0);
	if( *tty_in == -1){
		fprintf(stderr, "Cannot duplicate stdini\n");
		exit(EXIT_FAILURE);
		return -1;
	}	

	if( fcntl(*tty_in, F_SETFL, O_NONBLOCK == -1)){
		fprintf(stderr, "Cannot set tty_in as nonblock stdin\n");
		exit(EXIT_FAILURE);
		return -1;
	}

	ret_val = tcgetattr(*tty_in, option_old);
	if( ret_val != 0 ){
		errno_saved = errno;
		printf("error getting term setting of stdin\n Errno:%d\n",errno_saved);
		exit(EXIT_FAILURE);
		return -1;
	}
	tcgetattr(*tty_in,&option_ttyin);
	if( ret_val != 0 ){
		errno_saved = errno;
		printf("error getting term setting of stdin\n Errno:%d\n",errno_saved);
		exit(EXIT_FAILURE);
		return -1;
	}
	option_ttyin.c_cflag &= ~CSIZE;
	option_ttyin.c_cflag |= CS8;
	option_ttyin.c_cflag |= ( CLOCAL | CREAD);
	option_ttyin.c_iflag &= ~( INLCR | ICRNL);
	option_ttyin.c_iflag &= ~(IXON | IXOFF );
	option_ttyin.c_iflag &= ~ISTRIP;
	option_ttyin.c_lflag &= ~( ICANON | ECHO | ECHOE | ISIG);
	option_ttyin.c_cc[VMIN] = 0;
	option_ttyin.c_cc[VTIME] = 10;
	tcflush(*tty_in, TCIFLUSH);
	if(tcsetattr( *tty_in, TCSANOW, &option_ttyin) != 0){
		perror( "setting tty_in error");
		exit(EXIT_FAILURE);
		return -1;
	}
	return 0;
}
