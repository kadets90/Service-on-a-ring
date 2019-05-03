#ifndef Server_staff

	Item sign_up(int, char**);
	int get_porto(Item);
	char* get_csip(Item);
	char* get_ip(Item);
	int get_id1(Item);
	int get_id2(Item);
	int get_porto_TCP(Item);
	int get_porto_UDP(Item);
	char* param_reading(char*, char**, int);
	int server_interaction();
	char* message(int, Item, int);
	Item get_data(char*);
	int answer(Item);
	void free_ide(Item);
	int check_messageTCP(char*, Item);
	char* send_token(Item, char, Item);
	void start_go(int);
	int start_state();
	void operational_go(int);
	int operational_state();
	void avaible_go(int);
	int avaible_state();
	void UDP_msg(int, Item, int);
	void send_TCP_msg(char*, int);
	int token_decoder(char* , Item);
	void change_sucessor(int, int, char*, Item);
	void set_id2(Item, int);

#endif
