#ifndef SALA_H
#define SALA_H
#include <stdbool.h>
#include "raylib.h"

#define MAX_OBSTACULOS  16
#define MAX_INTERATIVOS 10

/* Paredes e portas (resolução fixa 1280x720, ver tela.h).
   Toda sala nasce com NUM_PAREDES obstáculos nas bordas, deixando uma
   abertura (porta) no meio das paredes esquerda e direita.
   Os obstáculos [0 .. NUM_PAREDES-1] são sempre as paredes. */
#define PAREDE_ESPESSURA 24
#define PORTA_Y          300   // topo da abertura da porta
#define PORTA_ALTURA     120
#define NUM_PAREDES      6

typedef enum {
    OBJ_BILHETE,
    OBJ_RELOGIO,      // relógio do salão: onde está a bomba (acusação + fios)
    // puzzle do salão (charada do bilhete)
    OBJ_PIANO,
    OBJ_TACA,
    OBJ_CARDAPIO,
    OBJ_REPERTORIO,
    // puzzle do jardim (quem está mentindo)
    OBJ_GARCONS,
    OBJ_FONTE,        // também abre o porão depois da galeria
    // puzzle da galeria (insertion sort)
    OBJ_QUADROS,
    // porão secreto
    OBJ_OTAVIO,
    // os colegas acrescentam aqui os ids dos objetos deles
} ObjetoId;

typedef struct {
    ObjetoId id;
    Rectangle area;
    const char *rotulo;   // texto que aparece no "[E] ..."
} Interativo;

typedef struct Sala{
    const char * nome;
    Color corFundo;
    Rectangle obstaculos[MAX_OBSTACULOS];
    int qtdObstaculos;
    struct Sala *proxSala;
    struct Sala *antSala;
    Interativo interativos[MAX_INTERATIVOS];
    int qntdInterativo;
} Sala;

Sala *salas_criar(void);           // monta o anel com as 3 salas e devolve a primeira
void  salas_liberar(Sala *inicio); // libera todos os nós
Sala *sala_ir_direita(Sala *atual);
Sala *sala_ir_esquerda(Sala *atual);
Sala *sala_inserir_depois(Sala *ref, const char *nome, Color cor);

/* Acrescentam um obstáculo / objeto interativo na sala.
   Devolvem false se o vetor já estiver cheio. */
bool  sala_adicionar_obstaculo(Sala *s, Rectangle r);
bool  sala_adicionar_interativo(Sala *s, ObjetoId id, Rectangle area, const char *rotulo);
#endif
