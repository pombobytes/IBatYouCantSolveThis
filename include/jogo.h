#ifndef JOGO_H
#define JOGO_H

#include "raylib.h" 
#include "sala.h"
#define ESCALA_MUNDO 3                    
#define VELOCIDADE_JOGADOR (120.0f * ESCALA_MUNDO)
#define TEMPO_TOTAL 600.0f
#define LARGURA_JOGADOR (12 * ESCALA_MUNDO)  
#define ALTURA_JOGADOR  (8 * ESCALA_MUNDO)

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