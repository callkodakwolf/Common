void Set_RAW_Mode( struct termios* termios_p);
speed_t Set_Baud(long* baud);
int Set_Serial_Port(int fd, struct termios* termios_p,long* baud, int timeout);
