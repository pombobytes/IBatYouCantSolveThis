#ifndef JOGO_H
#define JOGO_H

#include "raylib.h"
#include "tela.h"
#include "final.h"   // já inclui ui.h, inventario_tela.h e jogador.h
#include "puzzles.h"
#define ESCALA_MUNDO 3                               // sprites de 32x32 desenhados 3x maiores
#define VELOCIDADE_JOGADOR (120.0f * ESCALA_MUNDO)
#define TEMPO_TOTAL 600.0f
#define LARGURA_JOGADOR (12 * ESCALA_MUNDO)   // hitbox só dos pés: o sprite é desenhado por cima
#define ALTURA_JOGADOR  (8 * ESCALA_MUNDO)

typedef enum {
    MENU, 
    EXPLORANDO, 
    DIALOGO, 
    PUZZLE,
    INVENTARIO,
    ACUSACAO,      // escolher o suspeito e marcar as provas (final.h)
    BOMBA,         // cortar os fios na ordem da linha do tempo (final.h)
    FURTIVIDADE,
    FIM,           // tela de final; qual final está em jogo->resultado
    PAUSADO        // tecla P: o relógio da bomba para (ideia da Leticia)
} EstadoJogo;

/* Telas dentro do estado MENU */
typedef enum { TELA_MENU_PRINCIPAL, TELA_MENU_CONTROLES, TELA_MENU_CREDITOS } TelaMenu;

typedef struct {
    EstadoJogo estado;
    EstadoJogo estadoAnterior;   // estado em que o jogo estava antes de pausar
    TelaMenu telaMenu;       // principal, controles ou créditos
    int opcaoMenu;           // botão selecionado: 0 começar, 1 controles, 2 créditos
    float tempoRestante;
    Jogador jogador;   // posição, sala atual e inventário do Bruno
    Sala *salas;
    int interativoProximo;   // índice do objeto perto do Bruno na sala atual, ou -1
    Dialogo dialogo;   // texto mostrado no estado DIALOGO
    bool poraoAberto;
    TelaInventario telaInventario;

    // puzzles e "a ordem importa" (Salão -> Jardim -> Galeria)
    PuzzleId puzzleAtual;       // qual puzzle está aberto no estado PUZZLE
    PuzzleJardim puzzleJardim;
    PuzzleGaleria puzzleGaleria;
    bool leuBilhete;            // a charada do salão está no verso do bilhete
    bool salaoResolvido;
    bool jardimResolvido;
    bool galeriaResolvida;
    bool jardimPenalizado;      // a penalidade de ordem só é aplicada uma vez
    bool galeriaPenalizada;

    // diálogo que, ao fechar, termina o jogo (ex.: confronto no porão)
    bool terminarAposDialogo;
    ResultadoFinal resultadoPendente;

    // fim de jogo
    bool relogioLiberado;     // o relógio (bomba) só abre depois de liberado
    Acusacao acusacao;
    Bomba bomba;
    bool acusouCerto;
    bool capturouNoPorao;     // final muito bom (furtividade no porão)
    ResultadoFinal resultado; // qual final mostrar no estado FIM
} Jogo;

/* Imagens do jogo: carregar uma vez depois do InitWindow e liberar antes do CloseWindow */
void jogo_carregar_recursos(void);
void jogo_liberar_recursos(void);

void jogo_iniciar(Jogo *jogo);
void jogo_atualizar(Jogo *jogo);
void jogo_desenhar(const Jogo *jogo);
void jogo_finalizar(Jogo *jogo);
void jogo_penalizar(Jogo *jogo, int minutosJogo);
void jogo_abrir_porao(Jogo *jogo);
void jogo_liberar_relogio(Jogo *jogo);   // chamar quando o último puzzle for resolvido
#endif