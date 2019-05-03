CC=gcc
CFLAGS=-g -Wall -pedantic -O3

service: main.o Server_Staff.o TCP.o UDP.o
	$(CC) $(CFLAGS) -o service main.o Server_Staff.o TCP.o UDP.o

main.o: main.c server.h Server_Staff.h TCP.h UDP.h
	$(CC) -c $(CFLAGS) main.c

Server_Staff.o: Server_Staff.c server.h Server_Staff.h TCP.h UDP.h
	$(CC) -c $(CFLAGS) Server_Staff.c

TCP.o: TCP.c server.h TCP.h
	$(CC) -c $(CFLAGS) TCP.c

UDP.o: UDP.c server.h UDP.h
	$(CC) -c $(CFLAGS) UDP.c

clean::
	rm -f *.o core a.out recserv *~
