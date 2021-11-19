#include "uteis.h"

#define BUFSZ 1024

int socketServidor;

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

bool adicionarPokemon(char* nomePokemon, Pokedex* pokedex){
    Pokemon* pokemon = buscaNaPokedex(nomePokemon, pokedex);

    if (pokemon != NULL){
        return false; //Já existe
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
    return true; //Por enquanto, pois ainda tenho que tratar quando já tem esse pokemon e não pode adicionar de novo
}

void removerPokemon(char* nomePokemon, Pokemon* pokemon, Pokedex* pokedex){
    if(pokemon->anterior != NULL){
        Pokemon* pokAnterior = pokemon->anterior;
        pokAnterior->proximo = pokemon->proximo;
    }
    else{
        pokedex->primeiro = pokemon->proximo;
    }

    if(pokemon->proximo == NULL){
        pokedex->ultimo = pokemon->anterior;
    }

    pokemon->proximo = NULL;
    pokemon->anterior = NULL;

    pokedex->numPokemons--;
}

void consultarPokedex(){

}

void trocarPokemon(){

}

void configurarServidor(int argc, char **argv){

// if (argc < 3) {
    //     usage(argc, argv);
    // }

    //
    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
        //usage(argc, argv); //Repensar
    }

    //Tenta abrir o socket do servidor
    socketServidor = socket(storage.ss_family, SOCK_STREAM, 0);
    if (socketServidor == -1) {
        logexit("socket");
    }

    //Permitir que o servidor reutilize portas
    int permitir = 1;
    if (0 != setsockopt(socketServidor, SOL_SOCKET, SO_REUSEADDR, &permitir, sizeof(int))) {
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(socketServidor, addr, sizeof(storage))) {
        logexit("bind");
    }

    if (0 != listen(socketServidor, 10)) {
        logexit("listen");
    }

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ); //"toString" do endereço binário
    printf("bound to %socketServidor, waiting connections\n", addrstr);

}

int aceitarConexaoCliente(){

        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int socketCliente = accept(socketServidor, caddr, &caddrlen);
        if (socketCliente == -1) {
            logexit("accept");
        }

        char caddrstr[BUFSZ];
        addrtostr(caddr, caddrstr, BUFSZ);
        printf("[log] connection from %socketServidor\n", caddrstr);

        return socketCliente;

}