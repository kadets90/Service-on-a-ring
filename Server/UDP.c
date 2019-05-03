#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#include <time.h>

#include "server.h"
#include "UDP.h"

//estruturas com os dados para as sockets de servidor e cliente
struct sockaddr_in serveraddr, clientaddr;


/******************************************************************************
 * get_socket_UDP_server()
 *
 * Arguments: portUDP - porto que ira receber as mensagens
 * Returns: inteiro com a indicação do socket criado
 * Side-Effects: criaçao de um socket
 *
 * Description: criação de um socket de função de servidor UDP com os dados
 *              preetendidos e retorna o seu identificador
 *
 *****************************************************************************/
int get_socket_UDP_server(int portUDP){

	int fd;

	fd=socket(AF_INET, SOCK_DGRAM,0);
	if(fd==-1){
		printf("%s\n", strerror(errno));
		exit(1);
	}

	memset((void*)&serveraddr, (int) '\0', sizeof(serveraddr));

	serveraddr.sin_family=AF_INET;
	serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
	serveraddr.sin_port=htons((u_short)portUDP);

	if(bind(fd, (struct sockaddr*)&serveraddr, sizeof(serveraddr))==-1){
		printf("%s\n", strerror(errno));
		exit(1);
	}

	return fd;
}	

/******************************************************************************
 * get_socket_UDP_client()
 *
 * Arguments: portTCP - porto que ira receber as mensagens
 *            csip - string com o ip
 * Returns: inteiro com a indicação do socket criado
 * Side-Effects: criaçao de um socket
 *
 * Description: criação de um socket com função de cliente UDP com os dados
 *              preetendidos e retorna o seu identificador
 *
 *****************************************************************************/
int get_socket_UDP_client(char* csip, int portUDP){

	int fd;
	struct hostent *hostptr;

	hostptr=gethostbyname(csip);

	fd=socket(AF_INET, SOCK_DGRAM,0);
	if(fd==-1) exit(1);

	memset((void*)&serveraddr, (int) '\0', sizeof(serveraddr));

	serveraddr.sin_family=AF_INET;
	serveraddr.sin_addr.s_addr=((struct in_addr *)(hostptr->h_addr_list[0]))->s_addr;
	serveraddr.sin_port=htons((u_short)portUDP);

	return fd;
}

/******************************************************************************
 * UDP_client()
 *
 * Arguments: key - string com a mensagem a enviar
 *            fd - identificador do socket a utilizar
 * Returns: string com a resposta recebida, ou mensagem de erro
 * Side-Effects: alocação de memoria para uma string
 *
 * Description: trata de enviar uma mensagem por UDP e tenta receber a resposta
 *              a mesma, retornando uma mensagem de erro caso nao seja possivel
 *
 *****************************************************************************/
char* UDP_client(char* key, int fd){

	int n, m, addrlen;
	char buffer[80];
	char* aux=NULL;

	//Time out inicialization 
	struct timeval time_out;
	time_out.tv_sec=TIMEOUT;
	time_out.tv_usec=0;

	if(setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &time_out, sizeof(time_out))<0){
		printf("%s\n", strerror(errno));
		exit(1);
	}

	printf("%s\n", key);

	//tenta se eviar a mensagem por duas vezes, com um tempo limite de espera por resposta de TIMEOUT segundos
	for(int i=0; i<2; ++i){

		if(i==1)
			printf("REENVIO DA MENSAGEM!\n");

		addrlen=sizeof (serveraddr);
		n=sendto(fd,key,strlen(key), 0, (struct sockaddr*)&serveraddr,addrlen);
		if(n==-1){
			printf("%s\n", strerror(errno));
			exit(1);
		}

		addrlen=sizeof (serveraddr);
		m=recvfrom (fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&serveraddr, (socklen_t*)&addrlen);
		if(m>=0) break;
	}

	//Caso nao seja recebida resposta, imprime-se uma mensagem de errro
	if(m==-1)
		sprintf(buffer, "%s", "ERRO UDP!");
	else
		buffer[m]='\0';

	printf("%s\n", buffer);

	aux=(char*)malloc(sizeof(char)*(strlen(buffer)+1));
	if(aux==NULL) exit(1);

	sprintf(aux, "%s", buffer);

	return aux;
}


/******************************************************************************
 * UDP_server()
 *
 * Arguments: state - estado do servidor, se esta a atender um cliente ou nao
 *            fd - identificador do socket a utilizar
 * Returns: inteiro com a indicação do estado do servidor
 * Side-Effects: none
 *
 * Description: receção das mensagens enviadas pelos clientes e respetiva 
 *              verificação do protocologo de comunicação
 *
 *****************************************************************************/
int UDP_server(int state, int fd){

	int addrlen, n;
	char buffer[80];

	addrlen=sizeof(serveraddr);
	n=recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientaddr, (socklen_t*)&addrlen);

	buffer[n]='\0';

	printf("client said: %s\n", buffer);

	//se estiver livre apenas pode atender a um pedido
	if((strcmp(buffer, "MY_SERVICE ON")==0)&&(state==FREE)){

		state=BUSY;
		sprintf(buffer, "YOUR_SERVICE ON");
	}
	//se estiver ocupado apenas pode terminar um pedido
	else if((strcmp(buffer, "MY_SERVICE OFF")==0)&&(state==BUSY)){

		state=FREE;
		sprintf(buffer, "YOUR_SERVICE OFF");
	}
	else{
		printf("MENSAGEM ENVIADA PELO CLIENTE MAL FORMATADA\n");
		exit(1);
	}


	addrlen=sizeof(serveraddr);
	n=sendto(fd,buffer,strlen(buffer)+1, 0, (struct sockaddr*)&clientaddr,addrlen);
	if(n==-1) exit (1);	

	printf("you said: %s\n", buffer);

	return state;
}


