#ifndef UTEIS_H
#define UTEIS_H

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <sys/types.h>

//---------------------------------------------------------------------------------
//ESTRUTURAS

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

//---------------------------------------------------------------------------------
//FUNÇÕES

Pokemon* buscaNaPokedex(char* nome, Pokedex* pokedex);
void deletaPokedex(Pokedex* pokedex);

bool adicionarPokemon(char* nome, Pokedex* pokedex);
void removerPokemon();
void consultarPokedex();
void trocarPokemon();

void configurarServidor(int argc, char **argv);

#endif;