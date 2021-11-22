all:
	gcc -Wall -c common.c
	gcc -Wall -c uteisClient.c 
	gcc -Wall -c uteisServer.c 
	gcc -Wall client.c uteisClient.c common.o -o client
	gcc -Wall server.c uteisServer.c common.o -o server

clean:
	rm common.o client server
