#include <math.h>
#include "jogador_anim.h"
#include "jogo.h"

#define LARGURA_FRAME    32       /* célula da folha: 128x160 / 4 colunas x 5 linhas */
#define ALTURA_FRAME     32
#define TOTAL_FRAMES     4
#define FPS_ANDAR        8.0f
#define ESCALA_JOGADOR   ESCALA_MUNDO
#define SPRITE_OLHA_DIREITA 1     /* 0 se o sprite lateral olha para a esquerda */
#define LIMITE_TELEPORTE 50.0f    /* salto grande = troca de sala */

#define LINHA_FRENTE          0
#define LINHA_FRENTE_DIAGONAL 1
#define LINHA_LADO            2
#define LINHA_COSTAS_DIAGONAL 3
#define LINHA_COSTAS          4

static Texture2D folha;
static Vector2   ultimaPosicao;
static int       linha = LINHA_FRENTE;
static int       frameAtual = 0;
static float     tempo = 0;
static bool      espelhar = false;

void jogador_anim_iniciar(Vector2 posicao) {
    folha = LoadTexture("assets/jogador.png");
    SetTextureFilter(folha, TEXTURE_FILTER_POINT);
    ultimaPosicao = posicao;
}

void jogador_anim_atualizar(Vector2 posicao, float dt) {
    float dx = posicao.x - ultimaPosicao.x;
    float dy = posicao.y - ultimaPosicao.y;
    ultimaPosicao = posicao;

    if (fabsf(dx) > LIMITE_TELEPORTE || fabsf(dy) > LIMITE_TELEPORTE) { dx = 0; dy = 0; }

    bool andando = (dx != 0 || dy != 0);

    if (andando) {
        int lado = (dx > 0) - (dx < 0);
        if (dy > 0)      linha = lado ? LINHA_FRENTE_DIAGONAL : LINHA_FRENTE;
        else if (dy < 0) linha = lado ? LINHA_COSTAS_DIAGONAL : LINHA_COSTAS;
        else             linha = LINHA_LADO;
        if (lado != 0) espelhar = ((lado > 0) != SPRITE_OLHA_DIREITA);

        tempo += dt;
        if (tempo >= 1.0f / FPS_ANDAR) {
            tempo -= 1.0f / FPS_ANDAR;
            frameAtual = (frameAtual + 1) % TOTAL_FRAMES;
        }
    } else {
        frameAtual = 0;
        tempo = 0;
    }
}

void jogador_anim_desenhar(Vector2 posicao, float larguraHitbox, float alturaHitbox) {
    Rectangle origem = {
        (float)(frameAtual * LARGURA_FRAME),
        (float)(linha * ALTURA_FRAME),
        (float)(espelhar ? -LARGURA_FRAME : LARGURA_FRAME),
        (float)ALTURA_FRAME
    };
    float larg = LARGURA_FRAME * ESCALA_JOGADOR;
    float alt  = ALTURA_FRAME  * ESCALA_JOGADOR;
    /* centraliza na hitbox e alinha os pés com a base dela */
    Rectangle destino = {
        posicao.x + (larguraHitbox - larg) / 2.0f,
        posicao.y + alturaHitbox - alt,
        larg, alt
    };
    DrawTexturePro(folha, origem, destino, (Vector2){0, 0}, 0.0f, WHITE);
}

void jogador_anim_finalizar(void) { UnloadTexture(folha); }