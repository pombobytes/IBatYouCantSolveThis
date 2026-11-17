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
    jogo->salas = salas_criar();
    jogador_iniciar(&jogo->jogador, jogo->salas,(Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f });
    jogo->interativoProximo = -1;
    jogo->dialogo.aberto = false;
    jogo->poraoAberto = false;
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
    Rectangle bruno = {jogo->jogador.posicao.x,jogo->jogador.posicao.y,LARGURA_JOGADOR, ALTURA_JOGADOR};
    for (int i = 0; i < jogo->jogador.salaAtual->qtdObstaculos; i++) {
        if (CheckCollisionRecs(bruno, jogo->jogador.salaAtual->obstaculos[i])) {
            return true;
        }
    }
    return false;
}

static int buscar_interativo_proximo(const Jogo *jogo) {
    // retângulo do Bruno "inflado" em DISTANCIA_INTERACAO para cada lado
    Rectangle alcance = {
        jogo->jogador.posicao.x - DISTANCIA_INTERACAO,
        jogo->jogador.posicao.y - DISTANCIA_INTERACAO,
        LARGURA_JOGADOR + 2 * DISTANCIA_INTERACAO,
        ALTURA_JOGADOR + 2 * DISTANCIA_INTERACAO
    };

    for (int i = 0; i < jogo->jogador.salaAtual->qntdInterativo; i++) {
        if (CheckCollisionRecs(alcance, jogo->jogador.salaAtual->interativos[i].area)) {
            return i;
        }
    }
    return -1;
}

static void jogo_interagir(Jogo *jogo) {
    const Interativo *obj = &jogo->jogador.salaAtual->interativos[jogo->interativoProximo];
    switch (obj->id) {
        case OBJ_BILHETE: {
            static const char *falas[] = {
                "(Lendo o bilhete) \"Uma bomba explodirá à meia-noite.\"",
                "TSC!",
                "Meia-noite... Tenho menos de uma hora. E não posso causar pânico entre os convidados.",
                "Quem escreveu isso sabe quem eu sou. Ou quer que eu jogue o jogo dele.",
                "Sem o traje, sem ajuda. Só eu e o que eu conseguir descobrir nesta mansão."
            };
            int qtdFalas = sizeof(falas) / sizeof(falas[0]);
            dialogo_abrir(&jogo->dialogo, "Bruno", falas, qtdFalas);

            if (jogador_coletar_fala(&jogo->jogador, "Bilhete: \"Uma bomba explodirá à meia-noite.\"", HORARIO(23, 0))) {
                aviso_mostrar("Pista adicionada à linha do tempo", 2.0f);
            }
            jogo->estado = DIALOGO;
            break;    
        }
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
    float yAntigo = jogo->jogador.posicao.y;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
    {
        jogo->jogador.posicao.y -= passo;
    }
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
    {
        jogo->jogador.posicao.y += passo;
    }
    if(colide_com_sala(jogo)){
        jogo->jogador.posicao.y = yAntigo;
    }

    float xAntigo = jogo->jogador.posicao.x;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
    {
        jogo->jogador.posicao.x += passo;
    }
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
    {
        jogo->jogador.posicao.x -= passo;
    }
    if(colide_com_sala(jogo)){
        jogo->jogador.posicao.x = xAntigo;
    }

    //troca de sala
    int larguraTela = GetScreenWidth();
    if (jogo->jogador.posicao.x + LARGURA_JOGADOR > larguraTela)
    {
        jogo->jogador.salaAtual = sala_ir_direita(jogo->jogador.salaAtual);
        jogo->jogador.posicao.x = 0;
    }
    if (jogo->jogador.posicao.x < 0)
    {
        jogo->jogador.salaAtual = sala_ir_esquerda(jogo->jogador.salaAtual);
        jogo->jogador.posicao.x = larguraTela - LARGURA_JOGADOR;
    }

    int alturaTela = GetScreenHeight();
    //borda de cima
    if(jogo->jogador.posicao.y < 0){
        jogo->jogador.posicao.y = 0;
    }

    //borda de baixo
    if((jogo->jogador.posicao.y + ALTURA_JOGADOR) > alturaTela){
        jogo->jogador.posicao.y = alturaTela - ALTURA_JOGADOR;
    }

    //interação
    jogo->interativoProximo = buscar_interativo_proximo(jogo);
    if(jogo->interativoProximo != -1 && IsKeyPressed(KEY_E)){
        jogo_interagir(jogo);
    }
    if(IsKeyPressed(KEY_I)) {
        inventario_tela_abrir(&jogo->telaInventario, false);
        jogo->estado = INVENTARIO;
    }

    //teste para abrir o Porão
    // if(IsKeyPressed(KEY_P)){
    //     jogo_abrir_porao(jogo);
    // }
}

static void atualizar_dialogo(Jogo *jogo) {
    if(dialogo_atualizar(&jogo->dialogo) ==  true){
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

static void atualizar_inventario(Jogo *jogo) {
    if(inventario_tela_atualizar(&jogo->telaInventario, &jogo->jogador.inventario) == true){
        jogo->estado = EXPLORANDO;
    }
}


void jogo_atualizar(Jogo *jogo)
{
    aviso_atualizar(GetFrameTime());
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
    case INVENTARIO:
        atualizar_inventario(jogo);
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
    DrawRectangle(0, 0, larguraTela, alturaTela, jogo->jogador.salaAtual->corFundo);

    // nome da sala no canto superior esquerdo
    DrawText(jogo->jogador.salaAtual->nome, 20, 20, 30, WHITE);
    
    //obstaculo desenho
    for (int i = 0; i < jogo->jogador.salaAtual->qtdObstaculos; i++) {
        DrawRectangleRec(jogo->jogador.salaAtual->obstaculos[i], DARKGRAY);
    }
    
    //interativo desenho
    for(int i = 0; i < jogo->jogador.salaAtual->qntdInterativo; i++){
        DrawRectangleRec(jogo->jogador.salaAtual->interativos[i].area, YELLOW);
    }

    
    
    // jogador (BRUNO)
    DrawRectangle((int)jogo->jogador.posicao.x, (int)jogo->jogador.posicao.y, LARGURA_JOGADOR, ALTURA_JOGADOR, BLUE);
    
    //interativo aviso
    if (jogo->interativoProximo != -1 && jogo->estado == EXPLORANDO) {
        const Interativo *obj =  &jogo->jogador.salaAtual->interativos[jogo->interativoProximo];
        const char *aviso = TextFormat("[E] %s", obj->rotulo);
        DrawText(aviso, (int)jogo->jogador.posicao.x, ((int)jogo->jogador.posicao.y - 25),  20, GOLD);
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
    dialogo_desenhar(&jogo->dialogo);
}


static void desenhar_inventario(const Jogo *jogo) {
    desenhar_explorando(jogo);
    inventario_tela_desenhar(&jogo->telaInventario, &jogo->jogador.inventario);
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
    case INVENTARIO:
        desenhar_inventario(jogo);
        break;
    default:
        break;
    }
    aviso_desenhar();
}

void jogo_finalizar(Jogo *jogo)
{
    jogador_finalizar(&jogo->jogador);
    salas_liberar(jogo->salas);
    jogo->salas = NULL;
    jogo->jogador.salaAtual = NULL;
}

void jogo_penalizar(Jogo *jogo, int minutosJogo) {
    float segundos = minutosJogo * TEMPO_TOTAL / 60.0f;
    jogo->tempoRestante -= segundos;
}

void jogo_abrir_porao(Jogo *jogo) {
    if(jogo->poraoAberto == true){
        return;
    }
    Sala *jardim = jogo->salas->proxSala;
    Sala *porao  = sala_inserir_depois(jardim, "Porão", (Color){ 12, 15, 25, 255 });
    if(porao != NULL){
        jogo->poraoAberto = true;
    }
}