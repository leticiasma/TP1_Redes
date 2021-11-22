#include "uteisServer.h"

//---------------------------------------------------------------------------------

int main(int argc, char **argv) {

    int socketServidor;

    configurarServidor(argc, argv, &socketServidor);

    bool serverConectado = true;

    Pokedex pokedex;
    pokedex.numPokemons = 0;
    pokedex.primeiro = NULL;
    pokedex.ultimo = NULL;

    while (serverConectado) {
        
        int socketCliente = aceitarConexaoCliente(&socketServidor);

        bool clienteConectado = true;

        while(clienteConectado){

            char buf[BUFSZ];
            memset(buf, 0, BUFSZ);

            //Número de bytes recebidos
            size_t count;
            unsigned total = 0;

            bool mensagemCompleta = false;

            /*Recebe pacotes da mensagemRecebida que o cliente enviou até eu receber 0 bytes no count ou 
            um \n no buffer, indicando final da mensagem*/
            while(1){

                count = recv(socketCliente, buf + total, BUFSZ - total, 0);

                if (count == 0){
                    clienteConectado = false;
                    break;
                }

                char barra_n[2] = "\n";
                for (int i=total; i<count+total; i++){

                    if (strcmp(&buf[i], barra_n)==0){
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

            fputs(buf, stdout);

            char mensagemRecebida[500] = ""; 
            char mensagemEnvio[500] = "";
            const char delimitadores[] = " \n";
            
            strcpy(mensagemRecebida, buf);         
            char *token = strtok(mensagemRecebida, delimitadores);

            if(strcmp(token, "add\0")==0){

                token = strtok(NULL, delimitadores);

                int numPokemonsAdd = 0;

                while (token != NULL){

                    if (numPokemonsAdd >= 4){
                        break;
                    }

                    numPokemonsAdd++;

                    if (strlen(mensagemEnvio)>0){
                        strcat(mensagemEnvio, " ");
                    }

                    if (validarNome(token)){

                        if (pokedex.numPokemons < 40){

                            bool adicionou = adicionarPokemon(token, &pokedex);

                            if (adicionou){
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

                if (validarNome(token)){

                    Pokemon* pokemon = buscaNaPokedex(token, &pokedex);

                    if (pokemon != NULL){

                        removerPokemon(pokemon, &pokedex);

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
            else if (strcmp(token, "list\0")==0){
                
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

                if(!validarNome(nome1) || !validarNome(nome2)){
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

                //Desconectar o cliente e o servidor
                clienteConectado = false;
                close(socketCliente);

                serverConectado = false;

            }
            else{

                //Desconectar o cliente
                clienteConectado = false;
                close(socketCliente);

            }
            
            if (clienteConectado){

                char barra_n[2] = "\n";

                for (int i=0; i<strlen(mensagemEnvio); i++){
                    if (strcmp(&mensagemEnvio[i], barra_n)==0){
                        break;
                    }
                }

                strcat(mensagemEnvio, "\n");
                count = send(socketCliente, mensagemEnvio, strlen(mensagemEnvio), 0);
                
                if (count != strlen(mensagemEnvio)) {
                    logexit("send");
                }

            }

        }  

    }

    deletaPokedex(&pokedex);

    exit(EXIT_SUCCESS);

}