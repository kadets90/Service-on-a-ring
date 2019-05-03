CC=gcc
CFLAGS=-g -Wall -pedantic -O3

reqserv: main.o Client_Staff.o UDP.o
	$(CC) $(CFLAGS) -o reqserv main.o Client_Staff.o UDP.o

main.o: main.c client.h Client_Staff.h UDP.h
	$(CC) -c $(CFLAGS) main.c

Client_Staff.o: Client_Staff.c client.h Client_Staff.h
	$(CC) -c $(CFLAGS) Client_Staff.c

UDP.o: UDP.c client.h UDP.h
	$(CC) -c $(CFLAGS) UDP.c

clean::
	rm -f *.o core a.out recserv *~
