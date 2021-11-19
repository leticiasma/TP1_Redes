#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024

typedef struct{
    char nome[10];

    struct Pokemon* anterior;
    struct Pokemon* proximo;
}Pokemon;

typedef struct {
    //char pokemons[40][10]; //Limite de 40 pokemons com nomes de até 10 caracteres
    int numPokemons;

    Pokemon* primeiro;
    Pokemon* ultimo;
}Pokedex;

void deletaPokedex(Pokedex* pokedex){
    if(pokedex->numPokemons == 0){
        free(pokedex);
    }
    else{
        Pokemon* pokemonAtual = pokedex->primeiro;
        Pokemon* proxPokemon;

        while(pokemonAtual != NULL){
            proxPokemon = pokemonAtual->proximo;

            free(pokemonAtual);
            pokemonAtual = proxPokemon;
        }
        
        free(pokedex);
    }
}

Pokemon* buscaNaPokedex(char* nome, Pokedex* pokedex){
    Pokemon* pokemonAtual = pokedex->primeiro;

    while(pokemonAtual != NULL){
        if (strcmp(pokemonAtual->nome, nome)==0){
            return pokemonAtual;
        }
        pokemonAtual = pokemonAtual->proximo;
    }
    return NULL;
}

//int adicionarPokemon(char nome[], Pokedex* pokedex){
bool adicionarPokemon(char* nome, Pokedex* pokedex){
    Pokemon* pokemon = buscaNaPokedex(nome, pokedex);

    if (pokemon != NULL){
        printf("Pokemon ja existe!\n");
        return false; //Já existe
    } 
    
    pokemon = malloc(sizeof(Pokemon));
    strcpy(pokemon->nome, nome);
    pokemon->anterior = NULL;
    pokemon->proximo = NULL;

    if (pokedex->primeiro == NULL){
        pokedex->primeiro = pokemon;
        pokedex->ultimo = pokemon;
    }
    else{
        pokedex->ultimo->proximo = pokemon;
        pokemon->anterior = pokedex->ultimo;
        pokedex->ultimo = pokemon;
    }

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
    pokedex.primeiro = NULL;
    pokedex.ultimo = NULL;


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
    printf("bound to %s, waiting connections\n", addrstr);

    bool serverConectado = true;

    //Enquanto o server está vivo
    while (serverConectado) {
        
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
            char mensagem[500] = ""; //Máximo
            strcpy(mensagem, buf);

            const char delimitadores[] = " \n"; //OLHAR AS OUTRAS FORMAS DE DAR ESPAÇO
            char *token = strtok(mensagem, delimitadores);

            char mensagemEnvio[500] = "";

            pokedex.numPokemons = 38;

            if(strcmp(token, "add\0")==0){ //Tratar depois add e mais nada, que deveria dar erro
                //printf("%s\n",token);
                token = strtok(NULL, delimitadores);
                while (token != NULL){
                    printf("Pokemon a ser added: %s\n",token);
                    //++numTokens; //Acho que incrementa antes de printar, por exemplo
                    
                    //printf("%s\n",token);

                    if (strlen(mensagemEnvio)>0){
                        strcat(mensagemEnvio, " ");
                    }

                    if (strlen(token)<=10){
                        printf("Menor que 10\n");
                        if (pokedex.numPokemons < 40){
                            printf("Pokedex nao cheia\n");
                            bool sucesso = adicionarPokemon(token, &pokedex);
                            printf("Tentou add!\n");

                            if (sucesso){
                                printf("Conseguiu add\n");
                                strcat(mensagemEnvio, token);
                                strcat(mensagemEnvio, " added");
                            }
                            else{
                                printf("Nao conseguiu add\n");
                                strcat(mensagemEnvio, token);
                                strcat(mensagemEnvio, " already exists");
                            }  
                        }
                        else{
                            printf("Pokedex cheia\n");
                            strcat(mensagemEnvio, "limit exceeded");
                        }
                    }
                    else{
                        printf("Maior que 10\n");
                        strcat(mensagemEnvio, "invalid message");                
                    }
                    
                    token = strtok(NULL, delimitadores);
                }
            }
            else if (strcmp(token, "remove\0")==0){
                token = strtok(NULL, delimitadores);

                if (strlen(token)<=10){
                    Pokemon* pokemon = buscaNaPokedex(token, &pokedex);

                    if (pokemon != NULL){
                        
                        if(pokemon->anterior != NULL){
                            Pokemon* pokAnterior = pokemon->anterior;
                            pokAnterior->proximo = pokemon->proximo;
                        }else{
                            pokedex.primeiro = pokemon->proximo;
                        }

                        if(pokemon->proximo == NULL){
                            pokedex.ultimo = pokemon->anterior;
                        }

                        pokemon->proximo = NULL;
                        pokemon->anterior = NULL;

                        pokedex.numPokemons--;

                        strcat(mensagemEnvio, token);
                        strcat(mensagemEnvio, " removed");
                    }
                    else{

                        strcat(mensagemEnvio, token);
                        strcat(mensagemEnvio, " does not exist");               
                    }     
    
                }
                else{
                    strcat(mensagemEnvio, "invalid message");               
                }
            }
            else if (strcmp(token, "list\0")==0){ //Tratar quando pede list e manda coisas aleatórias depois
                if(pokedex.numPokemons == 0){
                    strcat(mensagemEnvio, "none");
                }
                else{
                    Pokemon* pokemonAtual = pokedex.primeiro;

                    while(pokemonAtual != NULL){
                        if(pokemonAtual->proximo == NULL){
                            
                            strcat(mensagemEnvio, pokemonAtual->nome);
                            
                        }
                        else{
                            
                            strcat(mensagemEnvio, pokemonAtual->nome);
                            strcat(mensagemEnvio, " ");
 
                        }
                        pokemonAtual = pokemonAtual->proximo;
                    }
                }
            }
            else if (strcmp(token, "exchange\0")==0){
                //printf("%s\n",token);
                char nome1[500];
                char nome2[500];

                token = strtok(NULL, delimitadores);
                printf("TOKEN: %s\n", token);
                strcpy(nome1, token);
                token = strtok(NULL, delimitadores);
                printf("TOKEN: %s\n", token);
                strcpy(nome2, token);
                printf("nome2: %s\n",nome2);

                if(strlen(nome1)>10){
                    strcat(mensagemEnvio, "invalid message");
                }
                else if (strlen(nome2)>10){
                    strcat(mensagemEnvio, "invalid message");
                }
                else{
                    Pokemon* pokemonTrocado = buscaNaPokedex(nome1, &pokedex);
                    Pokemon* novoPokemon = buscaNaPokedex(nome2, &pokedex);

                    if (pokemonTrocado == NULL){
                        strcat(mensagemEnvio, nome1);
                        strcat(mensagemEnvio, " does not exist");
                    }
                    else if (novoPokemon != NULL){
                        strcat(mensagemEnvio, nome2);
                        strcat(mensagemEnvio, " already exists"); 
                    }
                    else{
                        strcpy(pokemonTrocado->nome, nome2);

                        strcat(mensagemEnvio, nome1);
                        strcat(mensagemEnvio, " exchanged");
                    }
                }                              
            }
            else if (strcmp(token, "kill\0")==0){
                clienteConectado = false;
                close(csock); //Fecha o servidor tbm mas não era para acontecer isso

                serverConectado = false;
            }
            else{
                //Desconectar o cliente
                clienteConectado = false;
                close(csock); //Fecha o servidor tbm mas não era para acontecer isso
            }
            
            if (clienteConectado){
                printf("Enviando msg\n");
                strcat(mensagemEnvio, "\n");
                count = send(csock, mensagemEnvio, strlen(mensagemEnvio)+1, 0); //TIRAR +1s por causa do \0 da monitora
                if (count != strlen(mensagemEnvio)+1) { //Não sei se precisa dessa parte
                    logexit("send");
                }
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