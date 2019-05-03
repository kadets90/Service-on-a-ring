#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"
#include "Client_Staff.h"

//estrutura com os campos necessarios ao funcionamento do cliente
typedef struct{

	char* csip;
	int cspt;
	int id;	

}client_input;


/******************************************************************************
 * client_interaction()
 *
 * Arguments: state - estado de disponibilidade do cliente
 * Returns: inteiro com o numero do servico
 * Side-Effects: none
 *
 * Description: le, controla e decide o que fazer com a mensagem que recebeu
 *              do utilizador a partir do teclado
 *
 *****************************************************************************/
int client_interaction(int state){

	int service=STAY;
	char input[80], line[80];

	while(service==STAY){

		if(!strcmp(fgets(line, 80, stdin),"\n")){
			if(state==FREE)
				printf("YOU CAN USE THE FOLLOWING COMANDS:\nrequest_service (number of the service) or rs (number of the service)\nexit\n\n");
			else
				printf("YOU CAN USE THE FOLLOWING COMANDS:\nterminate_service or ts\n\n");
		}
		sscanf(line, "%s %i", input, &service);

		//ordens que pode receber e respetivas limitaçoes de estado
		if(((strcmp(input,"terminate_service")==0)||(strcmp(input,"ts")==0))&&(state==BUSY)){
			service=LEAVE;

		}else if((strcmp(input,"exit")==0)&&(state==FREE)){
			service=EXIT;

		}else if(((strcmp(input,"request_service")!=0)&&(strcmp(input,"rs")!=0))||(service<=0)||(state==BUSY)){
			printf("Pedido Inválido\n");
			service=STAY;
		}
	}

	return service;
}


/******************************************************************************
 * client_interaction()
 *
 * Arguments: argc - numero de parametros de entrada do programa
 *            argv - tabela com os parametros de entada
 * Returns: ponteiro com os dados da estrutura client
 * Side-Effects: alocação de memoria de uma estrutura do tipo client_input
 *
 * Description: Funçao para criar estrutura client_input e preenche-la com 
 *              dados do argv (entrada do programa)
 *
 *****************************************************************************/
Item get_arguments_client(int argc, char** argv){

	char* palavra;
	client_input* data=(client_input*)malloc(sizeof(client_input));

	//endereço IP servidor central
	palavra = param_reading("-i", argv, argc);
	if(palavra==NULL){
		data->csip =(char*)malloc(sizeof(char)*25);
		strcpy(data->csip, "tejo.tecnico.ulisboa.pt");
	}else{
		data->csip =(char*)malloc(sizeof(char)*(strlen(palavra)+1));
		strcpy(data->csip, palavra);
	}

	//Porto UDP do servidor central
	palavra = param_reading("-p", argv, argc);
	if (palavra==NULL)
		data->cspt = 59000;
	else
		data->cspt = strtol(palavra, NULL , 10);

	return data;
}

// Funçao para buscar o Porto à estrutura
int get_porto(Item data){
	return (*(client_input*)data).cspt;
}

// Funçao para buscar o IP à estrutura
char* get_ip(Item data){
	return (*(client_input*)data).csip;
}

// // Funçao para buscar o parametro indicado no argV (por exemplo por -i, ou -p)
char* param_reading(char* letter, char** argv, int argc){
	int i=0;

	while(strcmp(argv[i++],letter) ){
		if(i==argc){
			return NULL;
		}
	}
	return (argv[i]);
}


/******************************************************************************
 * answer()
 *
 * Arguments: ans - string com a resposta do servidor
 * Returns: ponteiro com os dados da estrutura com os dados do servidor
 * Side-Effects: alocação de memoria de uma estrutura do tipo client_input
 *
 * Description: Funçao para criar estrutura client_input e preenche-la com 
 *              os dados relativos ao servidor de despacho
 *
 *****************************************************************************/
Item answer(char* ans){

	client_input* server_data=NULL;
	char ip[30], ok[3];

	const char s[2] = ";";
  	char *token;

	server_data=(client_input*)malloc(sizeof(client_input));
	if(server_data==NULL){
		printf("Erro na criacao do server_data!\n");
		exit(1);
	}

	sscanf(ans, "%s %i;%s", ok, &server_data->id, ip);

   	/* get the first token with ip*/
   	token=strtok(ip, s);

	server_data->csip =(char*)malloc(sizeof(char)*(strlen(token)+1));

	sscanf(token, "%s", server_data->csip);
    
	token=strtok(NULL, s);

	sscanf(token, "%i", &(server_data->cspt));

	return server_data;
}


/******************************************************************************
 * check()
 *
 * Arguments: data - estrutura com os dados do cliente de despacho
 * Returns: inteiro com a identificação de sucesso ou insucesso na obtenção de
 *          servidor de despacho
 * Side-Effects: none
 *
 * Description: verifica a resposta dada por o servidor central e conclui se 
 *              existe servidor de despacho disponivel ou nao
 *
 *****************************************************************************/
int check(Item data){

	int order;

	if((strcmp((*(client_input*)data).csip, "0.0.0.0")==0)&&((*(client_input*)data).cspt==0))
		order=1;

	else
		order=0;

	return order;
}


/******************************************************************************
 * free_data()
 *
 * Arguments: ide - estrutura do tipo client_input
 * Returns: none
 * Side-Effects: libertação de memoria de uma estrutura do tipo client_input
 *
 * Description: Funçao para libertar a memoria ocupada por uma estrutura do
 *              tipo client_input
 *
 *****************************************************************************/
void free_data(Item ide){

	if((*(client_input*)ide).csip!=NULL)
		free((*(client_input*)ide).csip);

	free(ide);
}
