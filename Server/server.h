
#ifndef server

	#define max(A,B) ((A)>=(B)?(A):(B))
	#define ON 1
	#define OFF 0

	#define STR_SIZE 128
	#define IP_SIZE 30
	#define Item void*
	#define PORT 59000

	#define LEAVE -1
	#define EXIT -2
	#define STAY -3

	#define FREE 1
	#define BUSY 0

	#define CLIENT 1
	#define SERVER 2

	#define SET_DS 1
	#define WITHDRAW_DS 2
	#define SET_START 3
	#define WITHDRAW_START 4
	#define GET_START 5
	#define NEW 6
	#define NEW_START 7
	#define TOKEN_S 8
	#define TOKEN_I 9
	#define TOKEN_T 10
	#define TOKEN_D 11
	#define TOKEN_N 12
	#define TOKEN_O 13

	void wait_something(Item, int, Item, Item, int, int);

#endif
