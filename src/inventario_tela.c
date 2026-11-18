#include <stdio.h>
#include "raylib.h"
#include "tela.h"
#include "inventario_tela.h"

void inventario_tela_abrir(TelaInventario *t, bool podeMarcar) {
    t->aberta = true;
    t->podeMarcar = podeMarcar;
    t->selecionada = 0;
}

bool inventario_tela_atualizar(TelaInventario *t, Inventario *inv) {
    if (!t->aberta) return false;
    int n = inv->quantidade;

    if (n > 0) {
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) t->selecionada = (t->selecionada + 1) % n;
        if (IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_W)) t->selecionada = (t->selecionada - 1 + n) % n;
        if (t->podeMarcar && (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E)))
            inventario_alternar_marca(inv, t->selecionada);

        // mouse: passar por cima seleciona a linha; clicar marca/desmarca (mesmas posições do desenho)
        Vector2 mouse = GetMousePosition();
        for (int i = 0; i < n; i++) {
            Rectangle linha = { 30, 110 + i * 34 - 6, TELA_LARGURA - 60, 30 };
            if (CheckCollisionPointRec(mouse, linha)) {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    t->selecionada = i;
                    if (t->podeMarcar) inventario_alternar_marca(inv, i);
                }
                break;
            }
        }
    }

    // na acusação só o ESPAÇO confirma (ESC/I não podem encerrar a acusação sem querer)
    bool fechar = t->podeMarcar ? IsKeyPressed(KEY_SPACE)
                                : (IsKeyPressed(KEY_I) || IsKeyPressed(KEY_ESCAPE));
    if (fechar) t->aberta = false;
    return fechar;
}

void inventario_tela_desenhar(const TelaInventario *t, const Inventario *inv) {
    if (!t->aberta) return;
    int largura = TELA_LARGURA, altura = TELA_ALTURA;

    DrawRectangle(0, 0, largura, altura, Fade(BLACK, 0.85f));
    DrawText("LINHA DO TEMPO", 40, 30, 28, GOLD);
    DrawText(t->podeMarcar ? "Setas: navegar  ENTER ou clique: marcar/desmarcar pista"
                           : "Setas: navegar  I: fechar", 40, 66, 16, LIGHTGRAY);

    if (inv->inicio == NULL) {
        DrawText("Nenhuma pista coletada ainda.", 40, 110, 20, GRAY);
        return;
    }

    int i = 0, y = 110;
    for (const Pista *p = inv->inicio; p != NULL; p = p->prox, i++, y += 34) {
        bool sel = (i == t->selecionada);
        if (sel) DrawRectangle(30, y - 6, largura - 60, 30, Fade(GOLD, 0.25f));
        char linha[256];
        snprintf(linha, sizeof(linha), "%s %02dh%02d  [%s]  %s",
                 p->marcada ? "[X]" : "[ ]", p->horario / 60, p->horario % 60,
                 p->sala ? p->sala : "", p->texto);
        DrawText(linha, 40, y, 18, sel ? WHITE : LIGHTGRAY);
    }
    if (t->podeMarcar) {
        char rod[64];
        snprintf(rod, sizeof(rod), "Marcadas: %d", inventario_contar_marcadas(inv));
        DrawText(rod, 40, altura - 40, 18, GOLD);
        // botão bem visível: o ESPAÇO envia a acusação inteira
        const char *enviar = "[ESPACO] ENVIAR ACUSACAO";
        int le = MeasureText(enviar, 24);
        DrawRectangle(largura - le - 72, altura - 56, le + 32, 40, Fade(MAROON, 0.9f));
        DrawText(enviar, largura - le - 56, altura - 48, 24, WHITE);
    }
}
