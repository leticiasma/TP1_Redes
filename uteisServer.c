#include "uteisServer.h"

//---------------------------------------------------------------------------------
//Pokemon

bool validarNome(char* nomePokemon){

    if (strlen(nomePokemon)>10){
        return false;
    }
    else{

        char letra;

        for (int i=0; i<strlen(nomePokemon); i++){

            letra = nomePokemon[i];

            if (!((letra >= 'a' && letra <= 'z') || (letra >= '0' && letra <= '9'))){
                return false;
            }

        }
    
    }

    return true;
}

bool adicionarPokemon(char* nomePokemon, Pokedex* pokedex){

    Pokemon* pokemon = buscaNaPokedex(nomePokemon, pokedex);

    //Caso o pokémon já exista
    if (pokemon != NULL){
        return false;
    } 
    
    pokemon = malloc(sizeof(Pokemon));
    strcpy(pokemon->nome, nomePokemon);
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

    return true;

}

void removerPokemon(Pokemon* pokemon, Pokedex* pokedex){

    if(pokemon->anterior != NULL){
        Pokemon* pokemonAnterior = pokemon->anterior;
        pokemonAnterior->proximo = pokemon->proximo;
    }
    else{
        pokedex->primeiro = pokemon->proximo;
    }

    if(pokemon->proximo == NULL){
        pokedex->ultimo = pokemon->anterior;
    }

    pokemon->proximo = NULL;
    pokemon->anterior = NULL;

    free(pokemon);
    pokedex->numPokemons--;

}

//---------------------------------------------------------------------------------
//Pokedex

Pokemon* buscaNaPokedex(char* nomePokemon, Pokedex* pokedex){

    Pokemon* pokemonAtual = pokedex->primeiro;

    while(pokemonAtual != NULL){
        if (strcmp(pokemonAtual->nome, nomePokemon)==0){
            return pokemonAtual;
        }
        pokemonAtual = pokemonAtual->proximo;
    }
    
    return NULL;

}

void deletaPokedex(Pokedex* pokedex){

    if(pokedex->numPokemons != 0){

        Pokemon* pokemonAtual = pokedex->primeiro;
        Pokemon* proxPokemon = NULL;

        while(pokemonAtual != NULL){
            proxPokemon = pokemonAtual->proximo;

            free(pokemonAtual);
            pokemonAtual = proxPokemon;
        }

        pokedex->primeiro = NULL;
        pokedex->ultimo = NULL;
        
    }

}

//---------------------------------------------------------------------------------
//Configurações do sistema

void configurarServidor(int argc, char **argv, int* socketServidor){

    if (argc < 3) {
            usageServidor(argc, argv);
        }

    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
        usageServidor(argc, argv); //Repensar
    }

    //Tenta abrir o socket do servidor
    *socketServidor = socket(storage.ss_family, SOCK_STREAM, 0);
    if (*socketServidor == -1) {
        logexit("socket");
    }

    //Permitir que o servidor reutilize portas
    int permitir = 1;
    if (0 != setsockopt(*socketServidor, SOL_SOCKET, SO_REUSEADDR, &permitir, sizeof(int))) {
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(*socketServidor, addr, sizeof(storage))) {
        logexit("bind");
    }

    if (0 != listen(*socketServidor, 10)) {
        logexit("listen");
    }

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ); //"toString" do endereço binário

}

int aceitarConexaoCliente(int* socketServidor){

        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int socketCliente = accept(*socketServidor, caddr, &caddrlen);
        if (socketCliente == -1) {
            logexit("accept");
        }

        char caddrstr[BUFSZ];
        addrtostr(caddr, caddrstr, BUFSZ);

        return socketCliente;

}

void usageServidor(int argc, char **argv) {
    printf("usageServidor: %socketServidor <v4|v6> <server port>\n", argv[0]);
    printf("example: %socketServidor v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}