#ifndef Client_Staff

	int client_interaction(int);
	Item get_arguments_client(int, char**);
	int get_porto(Item);
	char* get_ip(Item);
	char* param_reading(char*, char**, int);
	Item answer(char*);
	int check(Item);
	void free_data(Item);

#endif
