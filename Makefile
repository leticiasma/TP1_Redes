all:
	gcc -Wall -c common.c
	gcc -Wall -c uteis.c 
	gcc -Wall client.c uteis.c common.o -o client
	gcc -Wall server.c uteis.c common.o -o server

clean:
	rm common.o client server
