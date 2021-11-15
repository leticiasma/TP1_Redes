#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024

void usage(int argc, char **argv) {
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
        usage(argc, argv);
    }

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage))) {
        logexit("bind");
    }

    if (0 != listen(s, 10)) {
        logexit("listen");
    }

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ); //"toString" do endereço binário

    while (1) {
        printf("bound to %s, waiting connections\n", addrstr);
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1) {
            logexit("accept");
        }

        printf("Cliente conectado\n");
        bool clienteConectado = true;

        while(clienteConectado){
            char caddrstr[BUFSZ];
            addrtostr(caddr, caddrstr, BUFSZ);
            printf("[log] connection from %s\n", caddrstr);

            char buf[BUFSZ];
            memset(buf, 0, BUFSZ);

            size_t count;
            unsigned total = 0;
            bool mensagemCompleta = false;

            //Recebe pacotes da mensagem que o cliente enviou até eu receber um 0 no count ou um \n no buffer
            while(1){
                count = recv(csock, buf, BUFSZ - 1, 0);
                if (count == 0){
                    clienteConectado = false;
                    break;
                }

                for (int i=total; i<buf+total; i++){
                    if (buf[i] == '\n'){ //TALVEZ NÃO FUNCIONE
                        mensagemCompleta = true;
                        break;
                    }
                }
                total += count;
                printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf);

                if (mensagemCompleta){
                    break;
                }
            }
            //sprintf(buf, "remote endpoint: %.1000s\n", caddrstr);
            count = send(csock, buf, strlen(buf) + 1, 0);
            if (count != strlen(buf) + 1) {
                logexit("send");
            }
        }        
    }
    //close(csock);
    exit(EXIT_SUCCESS);
}