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
    FURTIVIDADE,
    PAUSADO              // o tempo não corre neste estado
} EstadoJogo;

typedef struct {
    EstadoJogo estado;
    EstadoJogo estadoAnterior;   // estado em que o jogo estava antes de pausar
    float tempoRestante;
    Vector2 posicaoJogador;
    Sala *salas;
    Sala *salaAtual;
    int interativoProximo;   // índice do objeto perto do Bruno na sala atual, ou -1
    const char *textoDialogo;   // texto mostrado no estado DIALOGO
    bool poraoAberto;
    bool mudo;               // true = som desligado (tecla M)
} Jogo;

void jogo_iniciar(Jogo *jogo);
void jogo_atualizar(Jogo *jogo);
void jogo_desenhar(const Jogo *jogo);
void jogo_finalizar(Jogo *jogo);
void jogo_penalizar(Jogo *jogo, int minutosJogo);
void jogo_abrir_porao(Jogo *jogo);
void jogo_alternar_pausa(Jogo *jogo);   // pausa / retoma (tecla P)
void jogo_alternar_mudo(Jogo *jogo);    // liga / desliga o som (tecla M)
#endif
