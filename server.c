#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <sys/types.h>

#include "uteis.h"

#define BUFSZ 1024

void usage(int argc, char **argv) {
    printf("usage: %socketServidor <v4|v6> <server port>\n", argv[0]);
    printf("example: %socketServidor v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {

    //ACHO QUE, SE FECHA O SERVIDOR, FECHA A POKEDEX

    configurarServidor(argc, argv);


    bool serverConectado = true;

    Pokedex pokedex;
    pokedex.numPokemons = 0;
    pokedex.primeiro = NULL;
    pokedex.ultimo = NULL;

    while (serverConectado) {
        
        int socketCliente = aceitarConexaoCliente();

        bool clienteConectado = true;

        while(clienteConectado){

            char buf[BUFSZ];
            memset(buf, 0, BUFSZ);

            size_t count;
            unsigned total = 0;

            bool mensagemCompleta = false;

            //Recebe pacotes da mensagemRecebida que o cliente enviou até eu receber um 0 no count ou um \n no buffer
            while(1){

                count = recv(socketCliente, buf, BUFSZ - 1, 0);

                if (count == 0){
                    clienteConectado = false;
                    break;
                }

                //char barra_n[2] = "\n";
                for (int i=total; i<count+total; i++){
                    if (strcmp(&buf[i], "\n")==0){
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
                break;
            }

            //printf("[msg] %socketServidor, %d bytes: %socketServidor\n", caddrstr, (int)total, buf);

            //int numTokens = 0;
            char mensagemRecebida[500] = ""; //Máximo
            char mensagemEnvio[500] = "";
            const char delimitadores[] = " \n"; //OLHAR AS OUTRAS FORMAS DE DAR ESPAÇO
            
            strcpy(mensagemRecebida, buf);         
            char *token = strtok(mensagemRecebida, delimitadores);

            if(strcmp(token, "add\0")==0){ //Tratar depois add e mais nada, que deveria dar erro

                token = strtok(NULL, delimitadores);

                while (token != NULL){
                    //++numTokens; //Acho que incrementa antes de printar, por exemplo
                    if (strlen(mensagemEnvio)>0){
                        strcat(mensagemEnvio, " ");
                    }

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

                char nome1[500];
                char nome2[500];

                token = strtok(NULL, delimitadores);
                strcpy(nome1, token);
                token = strtok(NULL, delimitadores);
                strcpy(nome2, token);

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
                close(socketCliente); //Fecha o servidor tbm mas não era para acontecer isso

                serverConectado = false;

            }
            else{
                //Desconectar o cliente
                clienteConectado = false;
                close(socketCliente); //Fecha o servidor tbm mas não era para acontecer isso
            }
            
            if (clienteConectado){

                strcat(mensagemEnvio, "\n");
                count = send(socketCliente, mensagemEnvio, strlen(mensagemEnvio)+1, 0); //TIRAR +1s por causa do \0 da monitora
                
                if (count != strlen(mensagemEnvio)+1) { //Não sei se precisa dessa parte
                    logexit("send");
                }

            }

        }  

    }

    exit(EXIT_SUCCESS);

}