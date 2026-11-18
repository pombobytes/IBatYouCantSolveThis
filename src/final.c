#include "raylib.h"
#include "tela.h"
#include "final.h"

const int PROVAS[NUM_PROVAS]   = { HORARIO(22,20), HORARIO(22,40), HORARIO(22,50) };
const int ORDEM_FIOS[NUM_FIOS] = { HORARIO(22,5), HORARIO(22,20), HORARIO(22,40), HORARIO(22,50) };

static const char *NOMES_SUSPEITOS[TOTAL_SUSPEITOS] = {
    "Helena Vasconcelos", "Otavio Prado", "Sr. Almeida"
};

/* ================= ACUSACAO ================= */

void acusacao_iniciar(Acusacao *a, Inventario *inv) {
    a->etapa = ACUS_ESCOLHER_SUSPEITO;
    a->suspeito = -1;
    a->acertou = false;
    a->tela.aberta = false;
    inventario_desmarcar_todas(inv);
    escolha_abrir(&a->escolha, "Quem plantou a bomba?", NOMES_SUSPEITOS, TOTAL_SUSPEITOS);
}

/* Percorre a lista: so vale se exatamente as 3 provas estiverem marcadas. */
bool acusacao_verificar(int suspeito, const Inventario *inv) {
    if (suspeito != CULPADO) return false;
    if (inventario_contar_marcadas(inv) != NUM_PROVAS) return false;
    for (int i = 0; i < NUM_PROVAS; i++) {
        const Pista *p = inventario_buscar_horario(inv, PROVAS[i]);
        if (p == NULL || !p->marcada) return false;
    }
    return true;
}

bool acusacao_atualizar(Acusacao *a, Inventario *inv) {
    switch (a->etapa) {
    case ACUS_ESCOLHER_SUSPEITO: {
        int s = escolha_atualizar(&a->escolha);
        if (s >= 0) {
            a->suspeito = s;
            a->etapa = ACUS_MARCAR_PISTAS;
            inventario_tela_abrir(&a->tela, true);
        }
        return false;
    }
    case ACUS_MARCAR_PISTAS:
        if (inventario_tela_atualizar(&a->tela, inv)) {
            a->acertou = acusacao_verificar(a->suspeito, inv);
            a->etapa = ACUS_CONCLUIDA;
            return true;
        }
        return false;
    default:
        return true;
    }
}

void acusacao_desenhar(const Acusacao *a, const Inventario *inv) {
    if (a->etapa == ACUS_ESCOLHER_SUSPEITO) escolha_desenhar(&a->escolha);
    else if (a->etapa == ACUS_MARCAR_PISTAS) {
        inventario_tela_desenhar(&a->tela, inv);
        DrawText(TextFormat("Acusando: %s - marque as %d provas", NOMES_SUSPEITOS[a->suspeito], NUM_PROVAS),
                 40, TELA_ALTURA - 70, 18, WHITE);
    }
}

/* ================= FIOS DA BOMBA ================= */

void bomba_iniciar(Bomba *b) {
    /* ordem na tela embaralhada de proposito; a certa vem de ORDEM_FIOS */
    Fio iniciais[NUM_FIOS] = {
        { "Luva",           HORARIO(22,50),      RED,    false },
        { "Fonte",          HORARIO(22,20),  BLUE,   false },
        { "Chave de corda", HORARIO(22,5), YELLOW, false },
        { "Moldura",        HORARIO(22,40), GREEN,  false },
    };
    for (int i = 0; i < NUM_FIOS; i++) b->fios[i] = iniciais[i];
    b->selecionado = 0;
    b->proximo = 0;
    b->erros = 0;
    b->desarmada = b->explodiu = false;
}

bool bomba_atualizar(Bomba *b) {
    if (b->desarmada || b->explodiu) return true;

    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) b->selecionado = (b->selecionado + 1) % NUM_FIOS;
    if (IsKeyPressed(KEY_LEFT)  || IsKeyPressed(KEY_A)) b->selecionado = (b->selecionado - 1 + NUM_FIOS) % NUM_FIOS;

    // mouse: passar por cima seleciona o fio; clicar corta (mesmas posições de bomba_desenhar)
    bool clicou = false;
    int espaco = (TELA_LARGURA - 80) / NUM_FIOS;
    for (int i = 0; i < NUM_FIOS; i++) {
        int x = 40 + i * espaco + espaco / 2;
        Rectangle area = { x - 50, 130, 100, 280 };
        if (CheckCollisionPointRec(GetMousePosition(), area) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            b->selecionado = i;
            clicou = true;
        }
    }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E) || clicou) {
        Fio *f = &b->fios[b->selecionado];
        if (f->cortado) return false;

        if (f->horario == ORDEM_FIOS[b->proximo]) {
            f->cortado = true;
            b->proximo++;
            if (b->proximo == NUM_FIOS) b->desarmada = true;
        } else {
            b->erros++;
            aviso_mostrar(b->erros < ERROS_MAX ? "Fio errado! Mais um erro e explode" : "BOOM", 2.0f);
            if (b->erros >= ERROS_MAX) b->explodiu = true;
        }
    }
    return b->desarmada || b->explodiu;
}

void bomba_desenhar(const Bomba *b) {
    int w = TELA_LARGURA;
    DrawText("DESARME A BOMBA", 40, 40, 28, RED);
    DrawText("Corte os fios na ordem da linha do tempo.  Setas: escolher  ENTER ou clique: cortar  I: ver pistas",
             40, 80, 16, LIGHTGRAY);
    DrawText(TextFormat("Erros: %d/%d", b->erros, ERROS_MAX), w - 160, 40, 22, b->erros ? RED : WHITE);

    int espaco = (w - 80) / NUM_FIOS;
    for (int i = 0; i < NUM_FIOS; i++) {
        int x = 40 + i * espaco + espaco / 2;
        const Fio *f = &b->fios[i];
        if (f->cortado) {
            DrawRectangle(x - 4, 140, 8, 90, f->cor);
            DrawRectangle(x - 4, 270, 8, 90, f->cor);
        } else {
            DrawRectangle(x - 4, 140, 8, 220, f->cor);
        }
        if (i == b->selecionado) DrawRectangleLines(x - 50, 130, 100, 240, WHITE);
        int lt = MeasureText(f->rotulo, 18);
        DrawText(f->rotulo, x - lt / 2, 380, 18, f->cortado ? GRAY : WHITE);
    }
}

/* ================= FINAIS ================= */

ResultadoFinal final_decidir(bool capturouNoPorao, bool acusouCerto, bool desarmou, bool deuMeiaNoite) {
    if (capturouNoPorao) return RESULTADO_MUITO_BOM;
    if (deuMeiaNoite || !desarmou) return RESULTADO_RUIM;
    return acusouCerto ? RESULTADO_BOM : RESULTADO_MEDIO;
}

void final_desenhar(ResultadoFinal f) {
    const char *titulo = "", *texto = "";
    Color cor = WHITE;
    switch (f) {
    case RESULTADO_MUITO_BOM:
        titulo = "FINAL MUITO BOM";
        texto  = "Bruno capturou Otavio no porao antes de ele acionar o detonador.";
        cor = SKYBLUE; break;
    case RESULTADO_BOM:
        titulo = "FINAL BOM";
        texto  = "A bomba foi desarmada e o culpado foi preso.";
        cor = GREEN; break;
    case RESULTADO_MEDIO:
        titulo = "FINAL MEDIO";
        texto  = "A bomba foi desarmada, mas a pessoa errada foi acusada. O culpado fugiu.";
        cor = ORANGE; break;
    case RESULTADO_RUIM:
        titulo = "FINAL RUIM";
        texto  = "Meia-noite. A bomba explodiu.";
        cor = RED; break;
    default: return;
    }
    int w = TELA_LARGURA, h = TELA_ALTURA;
    ClearBackground(BLACK);
    DrawText(titulo, w / 2 - MeasureText(titulo, 40) / 2, h / 2 - 60, 40, cor);
    DrawText(texto,  w / 2 - MeasureText(texto, 20) / 2, h / 2, 20, WHITE);
    DrawText("ENTER para voltar ao menu", w / 2 - MeasureText("ENTER para voltar ao menu", 16) / 2,
             h / 2 + 60, 16, GRAY);
}
