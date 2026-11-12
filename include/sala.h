#ifndef SALA_H
#define SALA_H
#define MAX_OBSTACULOS 10
#define MAX_INTERATIVOS 10
#include "raylib.h"

typedef enum {
    OBJ_BILHETE,
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

#endif