#ifndef JOGO_H
#define JOGO_H

#include "raylib.h" 
#include "sala.h"
#define VELOCIDADE_JOGADOR 250.0f
#define TEMPO_TOTAL 600.0f
#define LARGURA_JOGADOR 32
#define ALTURA_JOGADOR  48

typedef enum {
    MENU, 
    EXPLORANDO, 
    DIALOGO, 
    PUZZLE,
    INVENTARIO, 
    FINAL_BOM, 
    FINAL_MEDIO, 
    FINAL_RUIM,
    FINAL_MUITO_BOM,
    FURTIVIDADE
} EstadoJogo;

typedef struct {
    EstadoJogo estado;
    float tempoRestante;
    Vector2 posicaoJogador;
    Sala *salas;
    Sala *salaAtual;
    int interativoProximo;   // índice do objeto perto do Bruno na sala atual, ou -1
    const char *textoDialogo;   // texto mostrado no estado DIALOGO
    bool poraoAberto;
} Jogo;

void jogo_iniciar(Jogo *jogo);
void jogo_atualizar(Jogo *jogo);
void jogo_desenhar(const Jogo *jogo);
void jogo_finalizar(Jogo *jogo);
void jogo_penalizar(Jogo *jogo, int minutosJogo);
void jogo_abrir_porao(Jogo *jogo);
#endif