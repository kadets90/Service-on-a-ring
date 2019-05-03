#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#include "server.h"
#include "TCP.h"


//estruturas com os dados para as sockets de servidor e cliente
struct sockaddr_in serveraddr, clientaddr;


/******************************************************************************
 * get_socket_TCP_server()
 *
 * Arguments: portTCP - porto que ira receber as mensagens
 * Returns: inteiro com a indicação do socket criado
 * Side-Effects: criaçao de um socket
 *
 * Description: criação de um socket de função de servidor TCP com os dados
 *              preetendidos e retorna o seu identificador
 *
 *****************************************************************************/
int get_socket_TCP_server(int portTCP){

	int fd;

	fd=socket(AF_INET, SOCK_STREAM,0);
	if(fd==-1){
		printf("%s\n", strerror(errno));
		exit(1);
	}

	memset((void*)&serveraddr, (int) '\0', sizeof(serveraddr));

	serveraddr.sin_family=AF_INET;
	serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
	serveraddr.sin_port=htons((u_short)portTCP);

	if(bind(fd, (struct sockaddr*)&serveraddr, sizeof(serveraddr))==-1){
		printf("%s\n", strerror(errno));
		exit(1);
	}

	listen(fd,FILA);
		
	return fd;
}


/******************************************************************************
 * TCP_client()
 *
 * Arguments: portTCP - porto que ira receber as mensagens
 * Returns: inteiro com a indicação do socket criado
 * Side-Effects: criaçao de um socket
 *
 * Description: criação de um socket de função de cliente TCP com os dados
 *              preetendidos, cria a ligação ao servidor e retorna o
 *              identificador do socket para onde devem ser enviadas as
 *              mensagens
 *
 *****************************************************************************/
int TCP_client(char* buffer, int portTCP){

	int fd;
	struct hostent *start;

	start=gethostbyname(buffer);

	fd=socket(AF_INET, SOCK_STREAM, 0);
	if(fd==-1){
		printf("%s\n", strerror(errno));
		exit(1);
	}

	memset((void*)&serveraddr, (int) '\0', sizeof(serveraddr));

	serveraddr.sin_family=AF_INET;
	serveraddr.sin_addr.s_addr=((struct in_addr *)(start->h_addr_list[0]))->s_addr;
	serveraddr.sin_port=htons((u_short)portTCP);


	if(connect (fd, (struct sockaddr*)&serveraddr, sizeof(serveraddr))==-1){
		printf("%s\n", strerror(errno));
		exit(1);
	}

	return fd;
}

/******************************************************************************
 * TCP_server()
 *
 * Arguments: fd - identificaçao do socket de serviço
 * Returns: inteiro com a indicação do socket criado
 * Side-Effects: criaçao de um socket
 *
 * Description: aceita o pedido de conecção por parte de um cliente TCP
 *
 *****************************************************************************/
int TCP_server(int fd){

	int newfd;
	int addrlen;

	addrlen=sizeof(clientaddr);

	//Neste novo canal (newfd) é por onde ele vai receber e enviar as mensagens TCP
	newfd=accept(fd, (struct sockaddr*)&clientaddr, (socklen_t*)&addrlen);
	if(newfd==-1){
		printf("%s\n", strerror(errno));
		exit(1);
	}

	return newfd;
}
