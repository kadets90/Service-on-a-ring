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

/*************************************/
/*Estrutura com os campos necessarios*/
/* à identificação e funcionamento de*/
/* um servidor************************/
/*************************************/
typedef struct{

	int id1;
	int id2;
	char* ip;
	int upt;
	int tpt;
	char* csip;
	int cspt;

}server_data;

//Identificação do servidor como de servidor de arranque
int start=OFF;

//Identificador do estado do anel
int avaible=ON;

//Identificação de servidor de despacho
int operational=OFF;

/******************************************************************************
 * sign_up()
 *
 * Arguments: argc - numero de parametros de entrada do programa
 *            argv - tabela com os parametros de entada
 * Returns: void*
 * Side-Effects: alocação de memoria de uma estrutura do tipo server_data
 *
 * Description: verifica os parametros de entrada e preenche os varios campos
 *              da estrutura com os dados do servidor
 *
 *****************************************************************************/
Item sign_up(int argc, char** argv){

	char* palavra;

	/*alocaçao de memoria para a estrutura ide*/
	server_data* ide=(server_data*)malloc(sizeof(server_data));
	if(ide==NULL){
		printf("Erro na criacao do ide!\n");
		exit(1);
	}

	//ID
	palavra=param_reading("-n", argv, argc);
	ide->id1=strtol(palavra, NULL , 10);

	//IP
	palavra=param_reading("-j", argv, argc);
	ide->ip=(char*)malloc(sizeof(char)*(strlen(palavra)+1));
	strcpy(ide->ip, palavra);

	//Porto UDP
	palavra=param_reading("-u", argv, argc);
	ide->upt=strtol(palavra, NULL , 10);

	//Porto TCP
	palavra=param_reading("-t", argv, argc);
	ide->tpt=strtol(palavra, NULL , 10);

	//endereço IP servidor central
	palavra=param_reading("-i", argv, argc);
	if(palavra==NULL){
		ide->csip =(char*)malloc(sizeof(char)*(strlen("tejo.tecnico.ulisboa.pt")+1));
		strcpy(ide->csip, "tejo.tecnico.ulisboa.pt");

	}else{
		ide->csip =(char*)malloc(sizeof(char)*(strlen(palavra)+1));
		strcpy(ide->csip, palavra);

	}

	//Porto UDP do servidor central
	palavra=param_reading("-p", argv, argc);
	if(palavra==NULL){
		ide->cspt = PORT;

	}else{
		ide->cspt=strtol(palavra, NULL , 10);

	}

	ide->id2=-1;

	return ide;
}


/******************************************************************************
 * param_reading()
 *
 * Arguments: argc - numero de parametros de entrada do programa
 *            argv - tabela com os parametros de entada
 *            letter - identifica se é ip ou porto
 * Returns: char*
 * Side-Effects: none
 *
 * Description: verifica se os parametros relativos ao servidor central foram
 *              introduzidos
 *
 *****************************************************************************/
char* param_reading(char* letter, char** argv, int argc){

	int i=1;

	while(strcmp(argv[i++],letter) ){
		if(i==argc){
			//caso seja o ip
			if(strcmp(letter, "-i")==0){
				return NULL;

			//caso seja o porto
			}else if(strcmp(letter, "-p")==0){
				return NULL;

			}
			exit(1);
		} 
	}
	return (argv[i]);
}

// Funçao para buscar o Porto do servidor central à estrutura
int get_porto(Item data){
	return (*(server_data*)data).cspt;
}

// Funçao para buscar o IP do servidor central à estrutura
char* get_csip(Item data){
	return (*(server_data*)data).csip;
}

// Funçao para buscar o IP à estrutura
char* get_ip(Item data){
	return (*(server_data*)data).ip;
}

// Funçao para buscar o id1 à estrutura
int get_id1(Item data){
	return (*(server_data*)data).id1;
}

// Funçao para buscar o id2 à estrutura
int get_id2(Item data){
	return (*(server_data*)data).id2;
}

// Funçao para buscar o Porto TCP à estrutura
int get_porto_TCP(Item data){
	return (*(server_data*)data).tpt;
}

// Funçao para buscar o Porto UDP à estrutura
int get_porto_UDP(Item data){
	return (*(server_data*)data).upt;
}

//Funcao para fazer Set ao ID2 no new
void set_id2(Item data, int aux){
	(*(server_data*)data).id2=aux;
}



/******************************************************************************
 * server_interaction()
 *
 * Arguments: none
 * Returns: int
 * Side-Effects: none
 *
 * Description: leitura do teclado e verificaçao das insturçoes do utilizador
 *              retorna um inteiro que identifica o serviço
 *
 *****************************************************************************/
int server_interaction(){

	int service=STAY;
	char input[80], line[80];

	//apenas sai do ciclo se as ordens estiverem bem formatadas 
	while(service==STAY){

		if(!strcmp(fgets(line, 80, stdin),"\n"))
			printf("YOU CAN USE THE FOLLOWING COMANDS:\njoin (number of the service)\nshow_state\nexit\n\n");

		else{
			sscanf(line, "%s %i", input, &service);

			if(strcmp(input,"exit")==0){
				service=EXIT;

			}else if(strcmp(input,"show_state")==0){

				printf("SERVER IS FREE\n");

			}else if(strcmp(input,"join")!=0){
				printf("INVALID REQUEST\n");
				service=STAY;
			}
		}
	}

	return service;
}

/******************************************************************************
 * message()
 *
 * Arguments: order - ordem a enviar
 *            ide - ponteiro para a estrutura com os dados do servidor
 *            service - numero do servico
 * Returns: string com a ordem a enviar com os campos devidamente preenchidos
 * Side-Effects: allocação de memoria para uma string
 *
 * Description: escrita de uma string com a ordem a enviar com os devidos 
 *              campos necessarios à sua correta interpretação
 *
 *****************************************************************************/
char* message(int order,Item ide, int service){

	char* aux=NULL;

	//string a enviar
	aux=(char*)malloc(sizeof(char)*STR_SIZE);
	if(aux==NULL) exit(1);

	if(order==SET_DS){
		sprintf(aux, "SET_DS %i;%i;%s;%i", service, (*(server_data*)ide).id1, (*(server_data*)ide).ip, (*(server_data*)ide).upt);

	}
	else if(order==WITHDRAW_DS){
		sprintf(aux, "WITHDRAW_DS %i;%i", service, (*(server_data*)ide).id1);

	}
	else if(order==SET_START){
		sprintf(aux, "SET_START %i;%i;%s;%i", service, (*(server_data*)ide).id1, (*(server_data*)ide).ip, (*(server_data*)ide).tpt);

	}
	else if(order==WITHDRAW_START){
		sprintf(aux, "WITHDRAW_START %i;%i", service, (*(server_data*)ide).id1);

	}
	else if(order==GET_START){
		sprintf(aux, "GET_START %i;%i", service, (*(server_data*)ide).id1);

	}
	else if(order==NEW){
		sprintf(aux, "NEW %i;%s;%i\n", (*(server_data*)ide).id1, (*(server_data*)ide).ip, (*(server_data*)ide).tpt);
	}
	else if(order==NEW_START){
		sprintf(aux, "NEW_START\n");
	}

	return aux;
}


/******************************************************************************
 * get_data()
 *
 * Arguments: ans - string com a string recebida de outro servidor
 * Returns: Item - ponteiro para a estrutura com a informação de um servidor
 * Side-Effects: allocação de memoria para uma estrutura do tipo server_data
 *
 * Description: alocação da informação recebida de outro servidor, podendo 
 *              ser este o central ou nao
 *
 *****************************************************************************/
Item get_data(char* ans){

	char ip[30], ok[3];

	const char s[2] = ";";
  	char* token;
	server_data* mdata=NULL;

	mdata=(server_data*)malloc(sizeof(server_data));
	if(mdata==NULL){
		printf("ERROR IN MEMORY ALLOCATION!\n");
		exit(1);
	}

	mdata->ip=NULL;
	mdata->csip=NULL;
	mdata->upt=-1;
	mdata->cspt=-1;

	sscanf(ans, "%s %i;%i;%s", ok, &mdata->id1, &mdata->id2, ip);

   	token=strtok(ip, s);

	mdata->ip=(char*)malloc(sizeof(char)*IP_SIZE);
	if(mdata->ip==NULL){
		printf("ERROR IN MEMORY ALLOCATION!\n");
		exit(1);
	}

	strcpy(mdata->ip, token);
    
	token=strtok(NULL, s);

	sscanf(token, "%i", &mdata->tpt);

	return mdata;
}




/******************************************************************************
 * answer()
 *
 * Arguments: mdata - estrutura com a informaçao respetiva a uma mensagem
 *                    recebida
 * Returns: order - destinção se o servidor é o primeiro no anel ou nao
 * Side-Effects: none
 *
 * Description: analise da resposta recebida do central e consequente toma de
 *              decisão
 *
 *****************************************************************************/
int answer(Item mdata){

	int order;

	if((strcmp((*(server_data*)mdata).ip, "0.0.0.0")==0)&&((*(server_data*)mdata).id2==0)&&((*(server_data*)mdata).tpt==0))
		order=ON;

	else
		order=OFF;

	return order;
}


/******************************************************************************
 * free_ide()
 *
 * Arguments: ide - ponteiro para uma estrutura do tipo server_data
 * Returns: none
 * Side-Effects: liberta a memoria ocupada por uma estrutura do tipo server_data
 *
 * Description: libertação de memoria de uma estrutura com dados relativos a um
 *              servidor
 *
 *****************************************************************************/
void free_ide(Item ide){

	free((*(server_data*)ide).ip);

	//campo apenas utilizado quando se trata da informaçao do proprio
	if((*(server_data*)ide).csip!=NULL)
		free((*(server_data*)ide).csip);

	free(ide);
}



/******************************************************************************
 * check_messageTCP()
 *
 * Arguments: message - mensagem recebida de um servidor
 *            mdata - estrutura que ira guradar a informação relativa a
 *                    mensagem recebida
 * Returns: inteiro com o valor da macro da ordem pretendida
 * Side-Effects: none
 *
 * Description: destinção da ordem recebida e preenchimento dos campos com a 
 *              informação recebida
 *
 *****************************************************************************/
int check_messageTCP(char* message, Item mdata){

	char TCP_order[10];

	const char s[2] = ";";
  	char *aux;

	sscanf(message, "%s %i", TCP_order, &(*(server_data*)mdata).id1);

	if(strcmp(TCP_order, "NEW")==0){

		sscanf(message, "%s %i;%s", TCP_order, &(*(server_data*)mdata).id1, (*(server_data*)mdata).ip);

		(*(server_data*)mdata).ip=strtok((*(server_data*)mdata).ip, s);
		

		aux=strtok(NULL, s);
		sscanf(aux, "%i", &(*(server_data*)mdata).tpt);

		//Quando New, fazemos id2=id1 (supostamente nao preenchiamos o id2), mas precisamos porque quando vai enviar Token N
		// a funçao send_token() faz uso do id2 e nao do id1;
		set_id2(mdata, (*(server_data*)mdata).id1);

		return NEW;

	}else if(strcmp(TCP_order, "TOKEN")==0){
		//é necessario determinar qual é o token em causa
		return token_decoder(message, mdata);

	}else if(strcmp(TCP_order, "NEW_START")==0){

		return NEW_START;

	}

	return 0;
}


/******************************************************************************
 * send_token()
 *
 * Arguments: ide - informação relativa ao proprio servidor
 *            type - identificação do token
 *            mdata - informação com os dados do servidor a entrar/sucessor do
 *                    que vai sair
 * Returns: string com a mensagem formatada a enviar
 * Side-Effects: allocação de memoria para uma string
 *
 * Description: identificação e formatação do token a enviar
 *
 *****************************************************************************/
char* send_token(Item ide, char type, Item mdata){

	char* aux=NULL;
	char buffer[STR_SIZE];

	if(type=='S'||type=='I'||type=='T'||type=='D'){

		sprintf(buffer, "TOKEN %i;%c\n", (*(server_data*)ide).id1, type);

	}else if(type=='N'||type=='O'){

		sprintf(buffer, "TOKEN %i;%c;%i;%s;%i\n", (*(server_data*)ide).id1, type, (*(server_data*)mdata).id2, (*(server_data*)mdata).ip, (*(server_data*)mdata).tpt);

	}else{
		printf("TOKEN IN WRONG FORMAT\n");
		exit(1);
	}

	aux=(char*)malloc(sizeof(char)*(strlen(buffer)+1));
	if(aux==NULL) exit(1);

	strcpy(aux, buffer);

	return aux;
}

//alteração de estado enquanto servidor de arranque
void start_go(int x){

	start=x;
}

//retorna se é servidor de arranque ou nao
int start_state(){

	return start;
}

//alteração de estado enquanto servidor de despacho
void operational_go(int x){

	operational=x;
}

//retorna se é servidor de despacho ou nao
int operational_state(){

	return operational;
}

//alteração de estado de disponibilidade do anel
void avaible_go(int x){

	avaible=x;
}

//retorna se o anel esta disponivel ou nao
int avaible_state(){

	return avaible;
}


/******************************************************************************
 * UDP_msg()
 *
 * Arguments: ide - informação relativa ao proprio servidor
 *            order - informaçao com a ordem que se quer enviar ao central
 *            service - numero do servico em causa
 * Returns: none
 * Side-Effects: none
 *
 * Description: envio de uma mensagem ao servidor central
 *
 *****************************************************************************/
void UDP_msg(int order, Item ide, int service){

	char* aux=NULL;
	//varaivel auxiliar apenas utilizada para libertar a memoria alocada pela função UDP_client
	char* trash=NULL;
	int fdaux;

	//atualização do estado do servidor
	if(order==SET_START)
		start_go(ON);
	else if(order==WITHDRAW_START)
		start_go(OFF);
	else if(order==SET_DS)
		operational_go(ON);
	else if(order==WITHDRAW_DS)
		operational_go(OFF);

	//obtenção do socket
	fdaux=get_socket_UDP_client(get_csip(ide), get_porto(ide));
	//mensagem a enviar
	aux=message(order, ide, service);
	//envio da mensagem
	trash=UDP_client(aux, fdaux);
	//libertação de memoria e do socket
	close(fdaux);
	free(aux);
	if(!strcmp(trash, "ERRO UDP!")){
		free(trash);
		exit(1);
	}
	free(trash);

}


/******************************************************************************
 * send_TCP_msg()
 *
 * Arguments: aux - mensagem a enviar
 *            fd - socket de envio das mensagens
 * Returns: none
 * Side-Effects: libertação de memoria ocupada por uma string
 *
 * Description: envio de uma mensagem a outro servidor
 *
 *****************************************************************************/
void send_TCP_msg(char* aux, int fd){

	//envio da mensagem
	if(write(fd, aux, strlen(aux)+1)==-1){
		printf("%s\n", strerror(errno));
		exit(1);
	}

	//libertação de memoria 
	free(aux);
}


/******************************************************************************
 * token_decoder()
 *
 * Arguments: message - string recebida pelo servidor
 *            mdata - estrutura onde serao guardados os dados recebidos
 * Returns: inteiro com a macro que define a ordem recebida
 * Side-Effects: none
 *
 * Description: identificação e formatação do token recebido
 *
 *****************************************************************************/
int token_decoder(char* message, Item mdata){

	char TCP_order[10];
	int auxID1, auxID2;
	char auxiliar1[30];
	char token;
	const char s[2] = ";";
  	char *aux;

	sscanf(message, "%s %i;%c;%i;%s", TCP_order, &auxID1, &token, &auxID2, auxiliar1);

	//guardar o id do servidor que emitiu o token
	(*(server_data*)mdata).id1 = auxID1;

	if (token=='S'){

		return TOKEN_S;
	}else if(token=='T'){ 

		return TOKEN_T;
	}else if(token=='I'){

		return TOKEN_I;
	}else if(token=='D'){

		return TOKEN_D;
	}else{

		//Preenchimento do resto dos campos relativos a informação recebida
		(*(server_data*)mdata).id2=auxID2;
		aux=strtok(auxiliar1, s);

		strcpy((*(server_data*)mdata).ip, aux);

		aux=strtok(NULL, s);
		sscanf(aux, "%i", &(*(server_data*)mdata).tpt);
		if (token=='O'){
			return TOKEN_O;
		}
		if(token=='N'){
			return TOKEN_N;
		}
	}	

	return 0;
}


/******************************************************************************
 * change_sucessor()
 *
 * Arguments: newID - novo id do sucessor do servidor
 *            newTPT - novo porto tCP do sucessor do servidor
 *            newIP - novo Ip do sucessor do servidor
 *            next - estrutura onde sao guardados os dados relativos ao sucessor
 * Returns: none
 * Side-Effects: none
 *
 * Description: alteração dos dados relativos ao novo servidor sucessor
 *
 *****************************************************************************/
void change_sucessor(int newID, int newTPT, char* newIP, Item next){

	if((*(server_data*)next).ip!=newIP)
		strcpy((*(server_data*)next).ip, newIP);

	(*(server_data*)next).tpt = newTPT;
	(*(server_data*)next).id1 = newID;


}
