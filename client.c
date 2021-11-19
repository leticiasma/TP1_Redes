#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void usage(int argc, char **argv) {
	printf("usage: %s <server IP> <server port>\n", argv[0]);
	printf("example: %s 127.0.0.1 51511\n", argv[0]);
	exit(EXIT_FAILURE);
}

#define BUFSZ 1024

int main(int argc, char **argv) {
	if (argc < 3) {
		usage(argc, argv);
	}

	struct sockaddr_storage storage;
	if (0 != addrparse(argv[1], argv[2], &storage)) {
		usage(argc, argv);
	}

	int s;
	s = socket(storage.ss_family, SOCK_STREAM, 0);
	if (s == -1) {
		logexit("socket");
	}
	struct sockaddr *addr = (struct sockaddr *)(&storage);
	if (0 != connect(s, addr, sizeof(storage))) {
		logexit("connect");
	}

	char addrstr[BUFSZ];
	addrtostr(addr, addrstr, BUFSZ);

	printf("connected to %s\n", addrstr);

	char buf[BUFSZ];

    bool desconectar = false;
    bool mensagemCompleta = false;

    while(1){ //Conversa, que dura até eu mandar kill ou até o servidor morrer comigo apertando Ctrl C
        memset(buf, 0, BUFSZ); //Limpa o buffer
        printf("mensagem> "); //Acho que é só > e quando recebe é <, ver nos testes disponibilizados
        fgets(buf, BUFSZ-1, stdin);
        //size_t count = send(s, buf, strlen(buf)+1, 0); //O quanto de bytes enviei
        size_t count = send(s, buf, strlen(buf), 0); //O quanto de bytes enviei
        // if (count != strlen(buf)+1) {
        //     logexit("send"); //Limpa o buffer
        // }
        if (count != strlen(buf)) {
            logexit("send"); 
        }

        memset(buf, 0, BUFSZ);//Limpa o buffer
        unsigned total = 0;

        mensagemCompleta = false;
        
        while(1) { //Fica recebendo pacotes do servidor
            printf("ESPERANDO PACOTE\n");
            count = recv(s, buf + total, BUFSZ - total, 0); //buf + total é a posiçõa da memória no buffer na qual começará a ser gravada a mensagem recebida pelo recv com tamanho máximo BUFSZ - total
            if (count == 0) { //Isso significa que o servidor morreu ou me matou NO SERVER <= 0
                // Connection terminated.
                desconectar = true;
                break;
            }

            char barra_n[2] = "\n";
            for (int i=total; i<count+total; i++){
                if (strcmp(&buf[i], barra_n)==0){ //TALVEZ NÃO FUNCIONE
                    mensagemCompleta = true;
                    break;
                }
            }
            
            total += count;

            if(mensagemCompleta){
                break;
            }
        }
        printf("received %u bytes\n", total);
        puts(buf); //Imprime o que recebeu

        if (desconectar){
            //printf("desconectou");
            break;
        }
    }
	close(s); //Fecha o socket do cliente

	exit(EXIT_SUCCESS);
}