#include <stdlib.h>
#include "jogo.h"

static const char *titulo = "I Bat You Can't Solve It";
static const char *subtitulo = "Press ENTER";
static const int tamanhoFonte = 40;
static const int tamanhoFonteSub = 20;

void jogo_iniciar(Jogo *jogo)
{
    jogo->estado = MENU;
    jogo->tempoRestante = TEMPO_TOTAL;
    jogo->posicaoJogador.x = GetScreenWidth() / 2.0f;
    jogo->posicaoJogador.y = GetScreenHeight() / 2.0f;
    jogo->salas = salas_criar();
    jogo->salaAtual = jogo->salas;
}

static void atualizar_menu(Jogo *jogo)
{
    if (IsKeyPressed(KEY_ENTER))
    {
        jogo->estado = EXPLORANDO;
    }
}

static void atualizar_explorando(Jogo *jogo)
{
    float dt = GetFrameTime();
    float passo = VELOCIDADE_JOGADOR * dt;

    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
    {
        jogo->posicaoJogador.y -= passo;
    }
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
    {
        jogo->posicaoJogador.y += passo;
    }
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
    {
        jogo->posicaoJogador.x += passo;
    }
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
    {
        jogo->posicaoJogador.x -= passo;
    }

    int larguraTela = GetScreenWidth();
    if (jogo->posicaoJogador.x + LARGURA_JOGADOR > larguraTela)
    {
        jogo->salaAtual = sala_ir_direita(jogo->salaAtual);
        jogo->posicaoJogador.x = 0;
    }
    if (jogo->posicaoJogador.x < 0)
    {
        jogo->salaAtual = sala_ir_esquerda(jogo->salaAtual);
        jogo->posicaoJogador.x = larguraTela - LARGURA_JOGADOR;
    }

    jogo->tempoRestante -= dt;
    if (jogo->tempoRestante <= 0)
    {
        jogo->estado = FINAL_RUIM;
        jogo->tempoRestante = 0;
    }
}

void jogo_atualizar(Jogo *jogo)
{
    switch (jogo->estado)
    {
    case MENU:
        atualizar_menu(jogo);
        break;
    case EXPLORANDO:
        atualizar_explorando(jogo);
        break;
    default:
        break;
    }
}

static void desenhar_menu(const Jogo *jogo)
{
    (void)jogo;
    int larguraTela = GetScreenWidth();
    int alturaTela = GetScreenHeight();
    int larguraTexto = MeasureText(titulo, tamanhoFonte);

    int x = (larguraTela - larguraTexto) / 2;
    int y = (alturaTela - tamanhoFonte) / 2;

    DrawText(titulo, x, y, tamanhoFonte, WHITE);

    int larguraSub = MeasureText(subtitulo, tamanhoFonteSub);

    int xSub = (larguraTela - larguraSub) / 2;
    int ySub = y + 60;

    DrawText(subtitulo, xSub, ySub, tamanhoFonteSub, GRAY);
}

static void desenhar_explorando(const Jogo *jogo)
{
    int larguraTela = GetScreenWidth();
    int alturaTela = GetScreenHeight();

    // fundo com a cor da sala atual
    DrawRectangle(0, 0, larguraTela, alturaTela, jogo->salaAtual->corFundo);

    // nome da sala no canto superior esquerdo
    DrawText(jogo->salaAtual->nome, 20, 20, 30, WHITE);
    // jogador (BRUNO)
    DrawRectangle((int)jogo->posicaoJogador.x, (int)jogo->posicaoJogador.y, LARGURA_JOGADOR, ALTURA_JOGADOR, BLUE);

    float tempoPassado = TEMPO_TOTAL - jogo->tempoRestante;
    int minutos = tempoPassado * 60 / TEMPO_TOTAL;

    const char *relogio;
    if (minutos < 60)
    {
        relogio = TextFormat("23:%02d", minutos);
    }
    else
    {
        relogio = "00:00";
    }

    int tamanhoRelogio = 30;
    int larguraRelogio = MeasureText(relogio, tamanhoRelogio);
    Color corRelogio = WHITE;
    if (minutos >= 50)
    {
        corRelogio = RED;
    }
    DrawText(relogio, larguraTela - larguraRelogio - 20, 20, tamanhoRelogio, corRelogio);
}

void jogo_desenhar(const Jogo *jogo)
{
    switch (jogo->estado)
    {
    case MENU:
        desenhar_menu(jogo);
        break;
    case EXPLORANDO:
        desenhar_explorando(jogo);
        break;
    default:
        break;
    }
}

void jogo_finalizar(Jogo *jogo)
{
    salas_liberar(jogo->salas);
    jogo->salas = NULL;
    jogo->salaAtual = NULL;
}