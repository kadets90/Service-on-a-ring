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
#include "Server_Staff.h"
#include "UDP.h"
#include "TCP.h"


int main (int argc, char** argv) {

	char *csip;
	long int cspt;
	int service, ck, fdUDP;
        Item ide=NULL;
	Item mdata=NULL;
	Item next=NULL;
	char* aux=NULL;
	char* ans=NULL;
	int fd_next=0;
	int fdTCP=0;

	//identificação do servidor e do servidor central
	ide=sign_up(argc, argv);
	csip=get_csip(ide);
	cspt=get_porto(ide);

	//Inicialização de sockets
	fdTCP=get_socket_TCP_server(get_porto_TCP(ide));

	printf("SERVICE ON\n"); 

	while((ck=server_interaction())!=EXIT){

		service=ck;

		//Information about the server_start 
		aux=message(GET_START, ide, service);

		fdUDP=get_socket_UDP_client(csip, cspt);

		ans=UDP_client(aux, fdUDP);

		free(aux);

		if(!strcmp(ans, "ERRO UDP!")){
			free(ans);
			continue;
		}

		mdata=get_data(ans);
		next=get_data(ans);

		//Alterar o id do sucessor para o Id do Start Server, manter porto TCP a 0 para o caso de ser o primeiro servidor (Start)
		change_sucessor(get_id2(next), get_porto_TCP(next), get_ip(next), next);

		free(ans);

		start_go(answer(mdata));

		//First in the ring
		if(start_state()==ON){

			UDP_msg(SET_START, ide, service);

			//Ready to work
			UDP_msg(SET_DS, ide, service);

		}
		else{
			//estabelecimento de uma ligação TCP
			fd_next=TCP_client(get_ip(mdata), get_porto_TCP(mdata));
			send_TCP_msg(message(NEW, ide, service), fd_next);
		}

		//fornecimento do servico, ligação ao anel, interface do utilizador
		wait_something(ide, service, mdata, next, fd_next, fdTCP);

		free_ide(mdata);
		free_ide(next);
		close(fdUDP);

	}
	free_ide(ide);
	close(fdTCP);

	printf("SERVICE OFF\n"); 

	exit(0);
}

/******************************************************************************
 * wait_somethin()
 *
 * Arguments: ide- estrutura com a informaçao relativa ao próprio servidor
 *	      service - numero do serviço
 *	      mdata - estrutura com a informaçao respetiva à mensagem recebida
 *	      next - estrutura com a informaçao relativa ao servidor sucessor
 *            fd_next - socket de envio das mensagens para o sucessor
 *            fdTCP - socket de receção das mensagens
 * Returns: none
 * Side-Effects: none
 *
 * Description: Ciclo onde o programa aguarda por alguma interaçao, seja por 
 *		TCP, UDP ou pelo teclado e toma decisões consoante o que recebe
 *
 *****************************************************************************/
void wait_something(Item ide, int service, Item mdata, Item next, int fd_next, int fdTCP){

	int n;
	int quit=OFF;
	char buffer[STR_SIZE];
	fd_set rfds;
	int maxfd, counter;
	int state=FREE;
	int fd, afd;
	int turn=OFF;
	int ordem;
	int future_quit=OFF;

	printf("IN SERVICE %i\n", service);
	fd=get_socket_UDP_server(get_porto_UDP(ide));

	while(quit==OFF){

		//Variaveis em escuta
		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
		FD_SET(fd,&rfds);
		FD_SET(fdTCP, &rfds);
		maxfd=max(fd,fdTCP);

		if (turn==ON){
			FD_SET(afd, &rfds);
			maxfd=max(afd, maxfd);
		}

		counter=select(maxfd+1, &rfds, (fd_set*)NULL, (fd_set*)NULL, (struct timeval *)NULL);
		if(counter<=0){
			printf("%s\n", strerror(errno));
			exit(1);
		}//errror

		/*****************************************************************/
		/*Caso chegue mensagem UDP, atendimento de um pedido a um cliente*/
		/*****************************************************************/
		if(FD_ISSET(fd,&rfds)){
			state=UDP_server(state, fd);

			//Quando servidor está ligado a um cliente
			if(state==BUSY){
				UDP_msg(WITHDRAW_DS, ide, service);
				if(get_porto_TCP(next)!=0){
					send_TCP_msg(send_token(ide, 'S', NULL), fd_next);
				}
				else
					avaible_go(OFF);
			}
			//Quando o servidor está sozinho no anel
			else if(get_porto_TCP(next)==0){
					UDP_msg(SET_DS, ide, service);
					avaible_go(ON);
			}
			//Quando o anel está indisponivel
			else if(avaible_state()==OFF){
					send_TCP_msg(send_token(ide, 'D', NULL), fd_next);
					avaible_go(ON);
			}
		}
		/*************************************************************************/
		/*Caso chegue mensagem do teclado, interaçao do utilizador com o servidor*/
		/*************************************************************************/
		else if(FD_ISSET(0,&rfds)){

			if(!strcmp(fgets(buffer, STR_SIZE, stdin),"\n"))
				printf("YOU CAN USE THE FOLLOWING COMANDS:\nleave\nshow_state\n\n");
			sscanf(buffer, "%s", buffer);

			//Quando não tem cliente e recebe do teclado "leave"
			if((strcmp(buffer,"leave")==0)&&(state==FREE)){

				//Caso o servidor a sair tenha sucessor (não se encontre sozinho no anel)
				if(get_porto_TCP(next)!=0){

					//Necessario entregar a funçao de despacho a outro
					if(operational_state()==ON){
						UDP_msg(WITHDRAW_DS, ide, service);
						send_TCP_msg(send_token(ide, 'S', NULL), fd_next);
						future_quit=ON;
					}
					else{
						//Id2 do next é utilizado em send_token(), logo igualamo lo a id1 do next
						set_id2(next, get_id1(next));
						send_TCP_msg(send_token(ide, 'O', next), fd_next);
					}
				}
				//Caso o servidor a sair não tenha sucessor
				else{
					quit=ON;
					UDP_msg(WITHDRAW_DS, ide, service);
				}

				//Quando o servidor a sair for o de arranque
				if(start_state()==ON){

					UDP_msg(WITHDRAW_START, ide, service);
					if(get_porto_TCP(next)!=0)
						send_TCP_msg(message(NEW_START, ide, service), fd_next);
				}

			//Quando é pedido ao servidor o estado em que se encontra (disponivel/indisponivel)
			}else if(strcmp(buffer,"show_state")==0){
				if(state==FREE)
					printf("SERVER IS FREE\n");
				else
					printf("SERVER IS BUSY\n");
			}
		}

		/***********************************************************************/
		/*Caso haja um pedido de connect (criaçao de canal TCP) na socket fdTCP*/
		/***********************************************************************/
		else if(FD_ISSET(fdTCP,&rfds)){

				if(turn==ON)
					close(afd);

				afd=TCP_server(fdTCP);
				turn=ON;
		}

		/*****************************************************************************/
		/*Leitura de mensagens do canal TCP (só existe quando já há canal TCP criado)*/
		/*****************************************************************************/
		else if(FD_ISSET(afd, &rfds)){

			if((n=read(afd, buffer, STR_SIZE))!=0){
				if(n==-1){
					printf("%s\n", strerror(errno));
					exit(1);
				}
				buffer[n]='\0';
				printf("%s", buffer);

				//Leitura da mensagem que chegou ao canal TCP e definição da ordem a que se refere
				ordem=check_messageTCP(buffer, mdata);

				//Caso mensagem recebida ser NEW
				if(ordem==NEW){

					//Quando só se encontrava no anel um servidor
					if(get_porto_TCP(next)==0){
						//Alteração dos dados relativos ao Sucessor com os dados provenientes da mensagem NEW
						change_sucessor(get_id1(mdata), get_porto_TCP(mdata), get_ip(mdata), next);
						fd_next=TCP_client(get_ip(next), get_porto_TCP(next));

						//Caso o servidor de arranque ser unico no anel e estar ocupado
						if(avaible_state()==OFF){
							send_TCP_msg(send_token(ide, 'I', NULL), fd_next);
						}
					//Quando existe mais que um anel no servidor	
					}else{
						//Send token N
						send_TCP_msg(send_token(ide, 'N', mdata), fd_next);
					}	
				//Caso mensagem recebida ser um TOKEN N
				}else if(ordem==TOKEN_N){
					//Quando o ID do servidor sucessor é diferente do ID do servidor de arranque, envia o token N para o seu sucessor
					if(get_id1(next)!=get_id1(mdata)){
						send_TCP_msg(send_token(mdata, 'N', mdata), fd_next);
					//Quando ID do servidor sucessor for igual ao ID do servidor de arranque, chegou ao fim do anel e inclui o novo servidor
					}else{
						change_sucessor(get_id2(mdata), get_porto_TCP(mdata), get_ip(mdata), next);
						close(fd_next);
						fd_next=TCP_client(get_ip(next), get_porto_TCP(next));

						//Quando o anel não estava disponivel
						if(avaible_state()==OFF)
							send_TCP_msg(send_token(ide, 'I', NULL), fd_next);
					}

				//Caso o servidor recebe um TOKEN O 
				}else if(ordem==TOKEN_O){
					//Quando o token O chega ao servidor que enviou
					if(get_id1(ide)==get_id1(mdata)){
						quit=ON;
						close(fd_next);
					//Quando o token O ainda não chegou ao servidor que o enviou, envia o Token pelo anel
					}else if(get_id1(next)!=get_id1(mdata)){

						send_TCP_msg(send_token(mdata, 'O', mdata), fd_next);

					//Quando o token O chega ao servidor antecessor (do servidor que vai sair), este altera o seu sucessor e envia o token O
					}else{
						send_TCP_msg(send_token(mdata, 'O', mdata), fd_next);
						close(fd_next);

						//Caso passe a haver um só servidor no anel
						if(get_id1(ide)==get_id2(mdata)){
							change_sucessor(0, 0, "0", next);
						}
						else{
							change_sucessor(get_id2(mdata), get_porto_TCP(mdata), get_ip(mdata), next);
							fd_next=TCP_client(get_ip(next), get_porto_TCP(next));
						}

					}
				//Caso o receba um TOKEN S
				}else if(ordem==TOKEN_S){
					//Quando o token S volta ao servidor que o emitiu
					if(get_id1(ide)==get_id1(mdata)){
						send_TCP_msg(send_token(mdata, 'I', NULL), fd_next);
						avaible_go(OFF);
					}
					//Quando o token S chega ao primeiro servidor disponivel 
					else if(state==FREE){
						UDP_msg(SET_DS, ide, service);
						send_TCP_msg(send_token(mdata, 'T', NULL), fd_next);
					}
					//Quando o servidor não está disponivel, e reenvia o token S para o anel
					else
						send_TCP_msg(send_token(mdata, 'S', NULL), fd_next);

				//Caso receba a informaçao que foi achado um novo servidor disponivel
				}else if(ordem==TOKEN_T){
					if(get_id1(ide)!=get_id1(mdata)){
						send_TCP_msg(send_token(mdata, 'T', NULL), fd_next);
					}
					else{
						printf("WAS FOUND A NEW SERVER\n");
						if(future_quit==ON){
							//Id2 do next é utilizado em send_token(), logo igualamo lo a id1 do next
							set_id2(next, get_id1(next));
							send_TCP_msg(send_token(ide, 'O', next), fd_next);
						}
					}
				//Caso receba a informaçao que o anes se encontra indisponivel 
				}else if(ordem==TOKEN_I){
					//Troca a informaçao de disponibilidade do anel no servidor
					if(avaible_state()==ON){
						if(state==BUSY){
							avaible_go(OFF);
							send_TCP_msg(send_token(mdata, 'I', NULL), fd_next);
						}
						else
							send_TCP_msg(send_token(ide, 'D', NULL), fd_next);
					}else if(future_quit==ON){
						//Id2 do next é utilizado em send_token(), logo igualamo lo a id1 do next
						set_id2(next, get_id1(next));
						send_TCP_msg(send_token(ide, 'O', next), fd_next);
						avaible_go(ON);
					}
				//Caso receba a informaçao que o anel passou a estar disponivel
				}else if(ordem==TOKEN_D){
					if(avaible_state()==ON){
						if(get_id1(ide)==get_id1(mdata))
							UDP_msg(SET_DS, ide, service);
						else if(get_id1(ide)>get_id1(mdata))
							send_TCP_msg(send_token(mdata, 'D', NULL), fd_next);
					}
					else{
						avaible_go(ON);
						send_TCP_msg(send_token(mdata, 'D', NULL), fd_next);
					}

				//Caso receba a informaçao que o servidor de arranque vai sair e é necessario arranjar novo servidor de arranque
				}else if(ordem==NEW_START){

					UDP_msg(SET_START, ide, service);
				}
			}

		}

	}
	close(fd);
	printf("OUT OF SERVICE %i\n", service);
}
