#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#include "client.h"
#include "UDP.h"
#include "Client_Staff.h"

int main (int argc, char** argv){

	int service=0;
	char *csip;
	long int cspt;
	char buffer[128];
	Item client_in=NULL;
	Item server_data=NULL;
	int state=FREE;
	char* ans=NULL;
	int fd;

	//leitura de argumentos de entrada e determinação de porto e ip do servidor central
	client_in=get_arguments_client(argc, argv);
	csip=get_ip(client_in);
	cspt=get_porto(client_in);

	while((service=client_interaction(state))!=EXIT){

		sprintf(buffer, "GET_DS_SERVER %i", service);

		fd=get_socket(csip, cspt);
		//envio de mensgem por UDP ao servidor central
		ans=UDP_client(buffer, fd);
		close(fd);

		//caso nao tenha sido possivel estabelecer ligaçao
		if(!strcmp(ans, "ERRO UDP!")){
			free(ans);
			continue;
		}

		//verificação e alocação de dados da resposta do servidor central
		server_data=answer(ans);

		free(ans);

		//verificar se existe servidor de despacho ou nao
		if(check(server_data))
			continue;

		while(service!=LEAVE){

			fd=get_socket(get_ip(server_data), get_porto(server_data));
			ans=UDP_client("MY_SERVICE ON", fd);
			if(!strcmp(ans, "ERRO UDP!")){
				free(ans);
				close(fd);
				break;
			}
			state=BUSY;
			free(ans);

			service=client_interaction(state);

			ans=UDP_client("MY_SERVICE OFF", fd);
			free(ans);
			close(fd);

			state=FREE;

		}
		free_data(server_data);
	}
	free_data(client_in);
	

	exit(0);
}
