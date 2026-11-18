#include <string.h>
#include "raylib.h"
#include "tela.h"
#include "puzzles.h"

#define QTD(v) ((int)(sizeof(v) / sizeof((v)[0])))

/* 
   JARDIM: quem está mentindo?
   Lógica: se o Davi mentisse, o Fábio ("tudo o que o Davi disse é
   verdade") também mentiria -> seriam dois mentirosos. Então o Davi e o
   Fábio dizem a verdade, e quem mente é o Edu (que acusa o Davi).
 */

#define MENTIROSO 2   // índice do Edu nas opções

static const char *DEPOIMENTOS[] = {
    "Exatamente UM dos quatro garçons está mentindo. Ouça com atenção.",
    "Caio: Às 22h10 a Sra. Helena saiu da galeria para atender um telefonema.",
    "Davi: Às 22h20 eu vi o Sr. Otávio mexendo na fonte do jardim.",
    "Edu: O Davi está mentindo. Ninguém chegou perto da fonte a noite toda.",
    "Fábio: Tudo o que o Davi disse é verdade. Eu estava com ele."
};

static const char *GARCONS[] = { "Caio", "Davi", "Edu", "Fábio" };

void jardim_iniciar(PuzzleJardim *p) {
    p->etapa = JARDIM_DEPOIMENTOS;
    p->escolha.aberta = false;
    dialogo_abrir(&p->dialogo, "Garçons", DEPOIMENTOS, QTD(DEPOIMENTOS));
}

ResultadoPuzzle jardim_atualizar(PuzzleJardim *p) {
    if (p->etapa == JARDIM_DEPOIMENTOS) {
        if (dialogo_atualizar(&p->dialogo)) {          // última fala fechada
            escolha_abrir(&p->escolha, "Quem está mentindo?", GARCONS, QTD(GARCONS));
            p->etapa = JARDIM_ESCOLHA;
        }
        return PZ_RODANDO;
    }

    int r = escolha_atualizar(&p->escolha);
    if (r < 0) return PZ_RODANDO;
    return (r == MENTIROSO) ? PZ_ACERTOU : PZ_ERROU;
}

void jardim_desenhar(const PuzzleJardim *p) {
    if (p->etapa == JARDIM_DEPOIMENTOS) dialogo_desenhar(&p->dialogo);
    else                                escolha_desenhar(&p->escolha);
}


//GALERIA: quadros do leilão (insertion sort)

/* Ordem inicial embaralhada de propósito. O mais antigo (1847) é o ano
   usado nos ladrilhos da fonte que abre o porão. */
static const Quadro QUADROS_INICIAIS[NUM_QUADROS] = {
    { "A Colheita",           1912 },
    { "Tempestade no Porto",  1888 },
    { "Dama de Azul",         1925 },
    { "Retrato do Fundador",  1847 },
    { "Jardim à Noite",       1901 },
    { "O Relojoeiro",         1869 },
};

/* Insertion sort clássico: para cada quadro, a partir do segundo, guarda
   ele numa variável, empurra para a direita os anteriores com ano maior
   e coloca o quadro guardado no buraco que sobrou. O(n^2). */
void quadros_insertion_sort(Quadro v[], int n) {
    for (int i = 1; i < n; i++) {
        Quadro atual = v[i];
        int j = i - 1;
        while (j >= 0 && v[j].ano > atual.ano) {
            v[j + 1] = v[j];
            j--;
        }
        v[j + 1] = atual;
    }
}

/* O movimento do jogador: tira o quadro da posição 'de' e insere na
   posição 'para', deslizando os quadros do meio (o mesmo deslocamento
   do insertion sort). */
void quadros_mover(Quadro v[], int n, int de, int para) {
    if (de < 0 || de >= n || para < 0 || para >= n || de == para) return;
    Quadro pego = v[de];
    if (de < para) {
        for (int k = de; k < para; k++) v[k] = v[k + 1];   // desliza para a esquerda
    } else {
        for (int k = de; k > para; k--) v[k] = v[k - 1];   // desliza para a direita
    }
    v[para] = pego;
}

// Confere a ordem do jogador comparando com uma cópia ordenada por insertion sort 
bool quadros_em_ordem(const Quadro v[], int n) {
    Quadro gabarito[NUM_QUADROS];
    if (n > NUM_QUADROS) n = NUM_QUADROS;
    memcpy(gabarito, v, n * sizeof(Quadro));
    quadros_insertion_sort(gabarito, n);
    for (int i = 0; i < n; i++)
        if (strcmp(v[i].titulo, gabarito[i].titulo) != 0) return false;
    return true;
}

void galeria_iniciar(PuzzleGaleria *p, bool destacarAntigo) {
    memcpy(p->quadros, QUADROS_INICIAIS, sizeof(QUADROS_INICIAIS));
    p->cursor = 0;
    p->pego = -1;
    p->movimentos = 0;
    p->destacarAntigo = destacarAntigo;
}

ResultadoPuzzle galeria_atualizar(PuzzleGaleria *p) {
    if (IsKeyPressed(KEY_ESCAPE)) {          // larga o quadro, ou sai do puzzle
        if (p->pego >= 0) { p->pego = -1; return PZ_RODANDO; }
        return PZ_SAIU;
    }
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) p->cursor = (p->cursor + 1) % NUM_QUADROS;
    if (IsKeyPressed(KEY_LEFT)  || IsKeyPressed(KEY_A)) p->cursor = (p->cursor - 1 + NUM_QUADROS) % NUM_QUADROS;

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E)) {
        if (p->pego < 0) {
            p->pego = p->cursor;                                   // pega o quadro
        } else {
            if (p->pego != p->cursor) {
                quadros_mover(p->quadros, NUM_QUADROS, p->pego, p->cursor);   // insere
                p->movimentos++;
            }
            p->pego = -1;
            if (quadros_em_ordem(p->quadros, NUM_QUADROS)) return PZ_ACERTOU;
        }
    }
    return PZ_RODANDO;
}

void galeria_desenhar(const PuzzleGaleria *p) {
    DrawRectangle(0, 0, TELA_LARGURA, TELA_ALTURA, Fade(BLACK, 0.85f));
    DrawText("OS QUADROS DO LEILÃO", 60, 80, 32, GOLD);
    DrawText("Coloque os quadros em ordem cronológica (do mais antigo ao mais novo).",
             60, 125, 20, LIGHTGRAY);
    DrawText(p->pego < 0 ? "Setas: escolher   ENTER: pegar quadro   ESC: sair": "Setas: escolher onde inserir   ENTER: inserir aqui   ESC: largar",60, 155, 18, GRAY);

    const int larg = 170, alt = 220, espaco = 25;
    const int total = NUM_QUADROS * larg + (NUM_QUADROS - 1) * espaco;
    const int x0 = (TELA_LARGURA - total) / 2, y0 = 240;

    for (int i = 0; i < NUM_QUADROS; i++) {
        const Quadro *q = &p->quadros[i];
        int x = x0 + i * (larg + espaco);
        int y = y0 - (i == p->pego ? 30 : 0); // o quadro pego fica "levantado"

        Color moldura = BROWN;
        if (p->destacarAntigo && q->ano == 1847) moldura = GOLD;   // dica do garçom
        if (i == p->pego) moldura = SKYBLUE;

        DrawRectangle(x, y, larg, alt, moldura);
        DrawRectangle(x + 10, y + 10, larg - 20, alt - 60, (Color){ 40, 40, 55, 255 });
        int tam = 16;   // diminui a fonte se o título não couber
        while (tam > 10 && MeasureText(q->titulo, tam) > larg - 12) tam--;
        int lt = MeasureText(q->titulo, tam);
        DrawText(q->titulo, x + (larg - lt) / 2, y + alt - 42, tam, BLACK);
        // placa com o ano
        const char *ano = TextFormat("%d", q->ano);
        DrawText(ano, x + (larg - MeasureText(ano, 20)) / 2, y0 + alt + 14, 20, WHITE);

        if (i == p->cursor) DrawRectangleLinesEx((Rectangle){ x - 6, y - 6, larg + 12, alt + 12 }, 4, WHITE);
    }
    if (p->destacarAntigo)
        DrawText("Dica do garçom: o quadro de moldura dourada é o mais antigo da coleção.",
                 60, TELA_ALTURA - 110, 18, GOLD);
    DrawText(TextFormat("Movimentos: %d", p->movimentos), 60, TELA_ALTURA - 70, 20, WHITE);
}
