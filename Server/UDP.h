#ifndef UDP

	#define TIMEOUT 3	//Time out UDP in seconds

	int get_socket_UDP_client(char*, int);
	int get_socket_UDP_server(int);
	char* UDP_client(char*, int);
	int UDP_server(int, int);

#endif
