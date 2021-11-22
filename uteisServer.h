#ifndef UTEIS_SERVER_H
#define UTEIS_SERVER_H

#include "common.h"

//---------------------------------------------------------------------------------
//ESTRUTURAS

typedef struct Pokemon{
    char nome[10];

    struct Pokemon* anterior;
    struct Pokemon* proximo;
}Pokemon;

typedef struct{
    //char pokemons[40][10]; //Limite de 40 pokemons com nomes de até 10 caracteres
    int numPokemons;

    Pokemon* primeiro;
    Pokemon* ultimo;
}Pokedex;

//---------------------------------------------------------------------------------
//FUNÇÕES

//Pokemon
bool validarNome(char* nomePokemon);
bool adicionarPokemon(char* nomePokemon, Pokedex* pokedex);
void removerPokemon(Pokemon* pokemon, Pokedex* pokedex);

//Pokedex
Pokemon* buscaNaPokedex(char* nomePokemon, Pokedex* pokedex);
void deletaPokedex(Pokedex* pokedex);

//Configurações do sistema
void configurarServidor(int argc, char **argv, int* socketServidor);
int aceitarConexaoCliente(int* socketServidor);
void usageServidor(int argc, char **argv);


#endif