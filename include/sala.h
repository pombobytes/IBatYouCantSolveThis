#ifndef SALA_H
#define SALA_H

#include "raylib.h"

typedef struct Sala {
    const char * nome;
    Color corFundo;
    struct Sala * proxSala;
    struct Sala * antSala;
} Sala;

Sala *salas_criar(void);           // monta o anel com as 3 salas e devolve a primeira
void  salas_liberar(Sala *inicio); // libera todos os nós
Sala *sala_ir_direita(Sala *atual);
Sala *sala_ir_esquerda(Sala *atual);

#endif