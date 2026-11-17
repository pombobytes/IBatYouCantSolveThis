#ifndef JOGO_H
#define JOGO_H

#include "raylib.h" 
#include "final.h"   // já inclui ui.h, inventario_tela.h e jogador.h
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
    ACUSACAO,      // escolher o suspeito e marcar as provas (final.h)
    BOMBA,         // cortar os fios na ordem da linha do tempo (final.h)
    FURTIVIDADE,
    FIM            // tela de final; qual final está em jogo->resultado
} EstadoJogo;

typedef struct {
    EstadoJogo estado;
    float tempoRestante;
    Jogador jogador;   // posição, sala atual e inventário do Bruno
    Sala *salas;
    int interativoProximo;   // índice do objeto perto do Bruno na sala atual, ou -1
    Dialogo dialogo;   // texto mostrado no estado DIALOGO
    bool poraoAberto;
    TelaInventario telaInventario;

    // fim de jogo
    bool relogioLiberado;     // o relógio (bomba) só abre depois de liberado
    Acusacao acusacao;
    Bomba bomba;
    bool acusouCerto;
    bool capturouNoPorao;     // final muito bom (furtividade no porão)
    ResultadoFinal resultado; // qual final mostrar no estado FIM
} Jogo;

void jogo_iniciar(Jogo *jogo);
void jogo_atualizar(Jogo *jogo);
void jogo_desenhar(const Jogo *jogo);
void jogo_finalizar(Jogo *jogo);
void jogo_penalizar(Jogo *jogo, int minutosJogo);
void jogo_abrir_porao(Jogo *jogo);
void jogo_liberar_relogio(Jogo *jogo);   // chamar quando o último puzzle for resolvido
#endif