#include <stdlib.h>
#include "jogo.h"

#define DISTANCIA_INTERACAO 20

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
    jogo->interativoProximo = -1;
    jogo->textoDialogo = NULL;
}

static void atualizar_menu(Jogo *jogo)
{
    if (IsKeyPressed(KEY_ENTER))
    {
        jogo->estado = EXPLORANDO;
    }
}

//colisao
static bool colide_com_sala(const Jogo *jogo) {
    Rectangle bruno = {jogo->posicaoJogador.x,jogo->posicaoJogador.y,LARGURA_JOGADOR, ALTURA_JOGADOR};
    for (int i = 0; i < jogo->salaAtual->qtdObstaculos; i++) {
        if (CheckCollisionRecs(bruno, jogo->salaAtual->obstaculos[i])) {
            return true;
        }
    }
    return false;
}

static int buscar_interativo_proximo(const Jogo *jogo) {
    // retângulo do Bruno "inflado" em DISTANCIA_INTERACAO para cada lado
    Rectangle alcance = {
        jogo->posicaoJogador.x - DISTANCIA_INTERACAO,
        jogo->posicaoJogador.y - DISTANCIA_INTERACAO,
        LARGURA_JOGADOR + 2 * DISTANCIA_INTERACAO,
        ALTURA_JOGADOR + 2 * DISTANCIA_INTERACAO
    };

    for (int i = 0; i < jogo->salaAtual->qntdInterativo; i++) {
        if (CheckCollisionRecs(alcance, jogo->salaAtual->interativos[i].area)) {
            return i;
        }
    }
    return -1;
}

static void jogo_interagir(Jogo *jogo) {
    const Interativo *obj = &jogo->salaAtual->interativos[jogo->interativoProximo];

    switch (obj->id) {
        case OBJ_BILHETE:
            jogo->textoDialogo = "Uma bomba explodirá à meia-noite.";
            jogo->estado = DIALOGO;
            //jogo_penalizar(jogo, 15); teste de penalização
            break;
        // acrescentem aqui o case do objeto de vcs
        default:
            break;
    }
}

static void atualizar_explorando(Jogo *jogo)
{
    float dt = GetFrameTime();
    float passo = VELOCIDADE_JOGADOR * dt;

    //movimentação
    float yAntigo = jogo->posicaoJogador.y;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
    {
        jogo->posicaoJogador.y -= passo;
    }
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
    {
        jogo->posicaoJogador.y += passo;
    }
    if(colide_com_sala(jogo)){
        jogo->posicaoJogador.y = yAntigo;
    }

    float xAntigo = jogo->posicaoJogador.x;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
    {
        jogo->posicaoJogador.x += passo;
    }
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
    {
        jogo->posicaoJogador.x -= passo;
    }
    if(colide_com_sala(jogo)){
        jogo->posicaoJogador.x = xAntigo;
    }

    //troca de sala
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

    int alturaTela = GetScreenHeight();
    //borda de cima
    if(jogo->posicaoJogador.y < 0){
        jogo->posicaoJogador.y = 0;
    }

    //borda de baixo
    if((jogo->posicaoJogador.y + ALTURA_JOGADOR) > alturaTela){
        jogo->posicaoJogador.y = alturaTela - ALTURA_JOGADOR;
    }

    //interação
    jogo->interativoProximo = buscar_interativo_proximo(jogo);
    if(jogo->interativoProximo != -1 && IsKeyPressed(KEY_E)){
        jogo_interagir(jogo);
    }


}

static void atualizar_dialogo(Jogo *jogo) {
    if(IsKeyPressed(KEY_E) || IsKeyPressed(KEY_ENTER)){
        jogo->estado = EXPLORANDO;
    }
}

//tempo
static void atualizar_tempo(Jogo *jogo) {
    float dt = GetFrameTime();
    jogo->tempoRestante -= dt;
    if (jogo->tempoRestante <= 0)
    {
        jogo->estado = FINAL_RUIM;
        jogo->tempoRestante = 0;
    }
}

void jogo_atualizar(Jogo *jogo)
{
    if (jogo->estado == EXPLORANDO || jogo->estado == DIALOGO || jogo->estado == PUZZLE || jogo->estado == INVENTARIO) {
        atualizar_tempo(jogo);
    }    
    switch (jogo->estado)
    {
    case MENU:
        atualizar_menu(jogo);
        break;
    case EXPLORANDO:
        atualizar_explorando(jogo);
        break;
    case DIALOGO:
        atualizar_dialogo(jogo);
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
    
    //obstaculo desenho
    for (int i = 0; i < jogo->salaAtual->qtdObstaculos; i++) {
        DrawRectangleRec(jogo->salaAtual->obstaculos[i], DARKGRAY);
    }
    
    //interativo desenho
    for(int i = 0; i < jogo->salaAtual->qntdInterativo; i++){
        DrawRectangleRec(jogo->salaAtual->interativos[i].area, YELLOW);
    }

    
    
    // jogador (BRUNO)
    DrawRectangle((int)jogo->posicaoJogador.x, (int)jogo->posicaoJogador.y, LARGURA_JOGADOR, ALTURA_JOGADOR, BLUE);
    
    //interativo aviso
    if (jogo->interativoProximo != -1 && jogo->estado == EXPLORANDO) {
        const Interativo *obj =  &jogo->salaAtual->interativos[jogo->interativoProximo];
        const char *aviso = TextFormat("[E] %s", obj->rotulo);
        DrawText(aviso, (int)jogo->posicaoJogador.x, ((int)jogo->posicaoJogador.y - 25),  20, GOLD);
    }
    
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

static void desenhar_dialogo(const Jogo *jogo) {
    desenhar_explorando(jogo);   // a sala continua aparecendo por trás

    int larguraTela = GetScreenWidth();
    int alturaTela  = GetScreenHeight();

    // caixa na parte de baixo da tela
    Rectangle caixa = { 40, alturaTela - 200, larguraTela - 80, 160 };
    DrawRectangleRec(caixa, (Color){ 0, 0, 0, 220 });   // preto quase opaco
    DrawRectangleLinesEx(caixa, 3, WHITE);              // borda

    DrawText(jogo->textoDialogo,(int)caixa.x+30,(int)caixa.y + 30, 28, WHITE);
    const char *continuar = "[E] continuar";
    int larguraContinuar = MeasureText(continuar, 18);
    DrawText(continuar, ((int)(caixa.x + caixa.width) - larguraContinuar - 20), ((int)(caixa.y + caixa.height) - 18 - 15), 18, GRAY);
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
    case DIALOGO:
        desenhar_dialogo(jogo);
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

void jogo_penalizar(Jogo *jogo, int minutosJogo) {
    float segundos = minutosJogo * TEMPO_TOTAL / 60.0f;
    jogo->tempoRestante -= segundos;
}