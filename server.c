#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024

typedef struct {
    char pokemons[40][10]; //Limite de 40 pokemons com nomes de até 10 caracteres
    int numPokemons;
}Pokedex;

//int adicionarPokemon(char nome[], Pokedex* pokedex){
bool adicionarPokemon(char* nome, Pokedex* pokedex){
    (*pokedex->pokemons)[pokedex->numPokemons] = nome;
    pokedex->numPokemons++;
    
    return true; //Por enquanto, pois ainda tenho que tratar quando já tem esse pokemon e não pode adicionar de novo
}

void usage(int argc, char **argv) {
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {

    //ACHO QUE, SE FECHA O SERVIDOR, FECHA A POKEDEX
    Pokedex pokedex;
    pokedex.numPokemons = 0;


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

        char caddrstr[BUFSZ];
        addrtostr(caddr, caddrstr, BUFSZ);
        printf("[log] connection from %s\n", caddrstr);

        printf("Cliente conectado\n");
        bool clienteConectado = true;

        while(clienteConectado){

            char buf[BUFSZ];
            memset(buf, 0, BUFSZ);

            size_t count;
            unsigned total = 0;
            bool mensagemCompleta = false;

            //Recebe pacotes da mensagem que o cliente enviou até eu receber um 0 no count ou um \n no buffer
            while(1){
                printf("ESPERANDO PACOTE\n");
                count = recv(csock, buf, BUFSZ - 1, 0);
                if (count == 0){
                    clienteConectado = false;
                    break;
                }

                char barra_n[2] = "\n";
                for (int i=total; i<count+total; i++){
                    printf("'%c'\n",buf[i]);
                    if (strcmp(&buf[i], barra_n)==0){ //TALVEZ NÃO FUNCIONE
                        mensagemCompleta = true;
                        break;
                    }
                }
                total += count;
                

                if (mensagemCompleta){
                    break;
                }
            }

            if(!clienteConectado){
                printf("CLIENTE DESCONECTADO\n");
                break;
            }

            printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)total, buf);

            //int numTokens = 0;
            char mensagem[500]; //Máximo
            strcpy(mensagem, buf);

            printf("passou 1\n");

            const char delimitadores[] = " \n"; //OLHAR AS OUTRAS FORMAS DE DAR ESPAÇO
            char *token = strtok(mensagem, delimitadores);

            printf("passou 2\n");

            if(strcmp(token, "add\0")==0){ //Tratar depois add e mais nada, que deveria dar erro
                printf("passou 3\n");
                //printf("%s\n",token);
                token = strtok(NULL, delimitadores);
                while (token != NULL){
                    //++numTokens; //Acho que incrementa antes de printar, por exemplo
                    
                    //printf("%s\n",token);
                    char mensagemEnvio[500];

                    if (strlen(token)<=10){
                        if (pokedex.numPokemons < 40){
                            bool sucesso = adicionarPokemon(token, &pokedex);

                            if (sucesso){
                                strcat(mensagemEnvio, token);
                                strcat(mensagemEnvio, " added");
                            }
                            else{
                                strcat(mensagemEnvio, token);
                                strcat(mensagemEnvio, " already exists");
                            }  
                        }
                        else{
                            strcat(mensagemEnvio, "limit exceeded");
                        }
                    }
                    else{
                        strcat(mensagemEnvio, "invalid message");                
                    }

                    count = send(csock, mensagemEnvio, strlen(mensagemEnvio)+1, 0); //TIRAR +1s por causa do \0 da monitora
                    if (count != strlen(mensagemEnvio)+1) { //Não sei se precisa dessa parte
                        logexit("send");
                    }
                    
                    token = strtok(NULL, delimitadores);
                }
            }
            else if (strcmp(token, "remove\0")==0){
                
            }
            else if (strcmp(token, "list\0")==0){
                
            }
            else if (strcmp(token, "exchange\0")==0){
                
            }
            else if (strcmp(token, "kill\0")==0){
                
            }
            else{
                //Desconectar o cliente
            }
            

            //sprintf(buf, "remote endpoint: %.1000s\n", caddrstr);
            
            // count = send(csock, buf, strlen(buf) + 1, 0);
            // if (count != strlen(buf) + 1) {
            //     logexit("send");
            // }
            
        }        
    }
    //close(csock);
    exit(EXIT_SUCCESS);
}