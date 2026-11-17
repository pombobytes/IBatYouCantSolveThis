#include <string.h>
#include "raylib.h"
#include "tela.h"
#include "ui.h"

/* Quebra o texto em linhas que cabem na largura e desenha. */
static void desenhar_texto_quebrado(const char *texto, int x, int y, int largura, int tam, Color cor) {
    char linha[256] = "";
    char palavra[64];
    int  yAtual = y;
    const char *p = texto;

    while (*p) {
        int k = 0;
        while (*p && *p != ' ' && k < 63) palavra[k++] = *p++;
        palavra[k] = '\0';
        while (*p == ' ') p++;

        char teste[256];
        strcpy(teste, linha);
        if (teste[0]) strcat(teste, " ");
        strcat(teste, palavra);

        if (MeasureText(teste, tam) > largura && linha[0]) {
            DrawText(linha, x, yAtual, tam, cor);
            yAtual += tam + 6;
            strcpy(linha, palavra);
        } else {
            strcpy(linha, teste);
        }
    }
    if (linha[0]) DrawText(linha, x, yAtual, tam, cor);
}

/* ================= DIALOGO (fila) ================= */

void dialogo_abrir(Dialogo *d, const char *quemFala, const char *falas[], int n) {
    if (n > MAX_FALAS) n = MAX_FALAS;
    for (int i = 0; i < n; i++) d->falas[i] = falas[i];   /* enfileira */
    d->quemFala = quemFala;
    d->inicio = 0;
    d->fim = n;
    d->aberto = (n > 0);
}

bool dialogo_atualizar(Dialogo *d) {
    if (!d->aberto) return false;
    if (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        d->inicio++;                       /* desenfileira */
        if (d->inicio >= d->fim) {
            d->aberto = false;
            return true;
        }
    }
    return false;
}

void dialogo_desenhar(const Dialogo *d) {
    if (!d->aberto) return;
    int w = TELA_LARGURA, h = TELA_ALTURA;
    Rectangle caixa = { 20, h - 160, w - 40, 140 };

    DrawRectangleRec(caixa, Fade(BLACK, 0.9f));
    DrawRectangleLinesEx(caixa, 2, GOLD);
    if (d->quemFala) DrawText(d->quemFala, caixa.x + 16, caixa.y + 10, 20, GOLD);
    desenhar_texto_quebrado(d->falas[d->inicio], caixa.x + 16, caixa.y + 40,
                          caixa.width - 32, 20, WHITE);
    DrawText("E >", caixa.x + caixa.width - 50, caixa.y + caixa.height - 28, 18, LIGHTGRAY);
}

/* ================= ESCOLHA ENTRE N OPCOES ================= */

void escolha_abrir(Escolha *e, const char *titulo, const char *opcoes[], int n) {
    if (n > MAX_OPCOES) n = MAX_OPCOES;
    e->titulo = titulo;
    for (int i = 0; i < n; i++) e->opcoes[i] = opcoes[i];
    e->n = n;
    e->selecionada = 0;
    e->aberta = (n > 0);
}

int escolha_atualizar(Escolha *e) {
    if (!e->aberta) return -1;
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) e->selecionada = (e->selecionada + 1) % e->n;
    if (IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_W)) e->selecionada = (e->selecionada - 1 + e->n) % e->n;
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E)) {
        e->aberta = false;
        return e->selecionada;
    }
    return -1;
}

void escolha_desenhar(const Escolha *e) {
    if (!e->aberta) return;
    int w = TELA_LARGURA;
    int alturaCaixa = 70 + e->n * 36;
    Rectangle caixa = { w / 2 - 220, 120, 440, alturaCaixa };

    DrawRectangleRec(caixa, Fade(BLACK, 0.92f));
    DrawRectangleLinesEx(caixa, 2, GOLD);
    DrawText(e->titulo, caixa.x + 20, caixa.y + 16, 22, GOLD);

    for (int i = 0; i < e->n; i++) {
        int y = caixa.y + 60 + i * 36;
        bool sel = (i == e->selecionada);
        if (sel) DrawRectangle(caixa.x + 10, y - 6, caixa.width - 20, 30, Fade(GOLD, 0.25f));
        DrawText(TextFormat("%s %s", sel ? ">" : " ", e->opcoes[i]), caixa.x + 20, y, 20,
                 sel ? WHITE : LIGHTGRAY);
    }
}

/* ================= AVISO ================= */

static char  avisoTexto[64] = "";
static float avisoTempo = 0;

void aviso_mostrar(const char *texto, float segundos) {
    strncpy(avisoTexto, texto, sizeof(avisoTexto) - 1);
    avisoTexto[sizeof(avisoTexto) - 1] = '\0';
    avisoTempo = segundos;
}

void aviso_atualizar(float dt) {
    if (avisoTempo > 0) avisoTempo -= dt;
}

void aviso_desenhar(void) {
    if (avisoTempo <= 0) return;
    int tam = 24, larg = MeasureText(avisoTexto, tam);
    int x = TELA_LARGURA / 2 - larg / 2;
    float alfa = avisoTempo < 0.5f ? avisoTempo / 0.5f : 1.0f;   /* some suavemente */
    DrawRectangle(x - 14, 20, larg + 28, 40, Fade(BLACK, 0.8f * alfa));
    DrawText(avisoTexto, x, 28, tam, Fade(GOLD, alfa));
}
