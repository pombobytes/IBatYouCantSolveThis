#include <stdlib.h>
#include <string.h>
#include "jogador.h"


/* ================= JOGADOR ================= */

void jogador_iniciar(Jogador *j, Sala *salaInicial, Vector2 posicao) {
    j->posicao = posicao;
    j->salaAtual = salaInicial;
    inventario_iniciar(&j->inventario);
}

void jogador_finalizar(Jogador *j) {
    inventario_limpar(&j->inventario);
}

bool jogador_coletar_fala(Jogador *j, const char *texto, int horario) {
    const char *sala = j->salaAtual ? j->salaAtual->nome : "";
    return inventario_inserir(&j->inventario, texto, horario, sala);
}

/* ================= INVENTÁRIO: lista encadeada simples ================= */

void inventario_iniciar(Inventario *inv) {
    inv->inicio = NULL;
    inv->quantidade = 0;
}

/* Inserção ordenada por horário: anda até o primeiro nó com horário maior.
   É o mesmo passo do insertion sort, aplicado a uma lista.
   Não insere pista repetida (mesmo texto). O(n). */
bool inventario_inserir(Inventario *inv, const char *texto, int horario, const char *sala) {
    if (inventario_buscar(inv, texto) != NULL) return false;

    Pista *nova = malloc(sizeof(Pista));
    if (nova == NULL) return false;
    nova->texto = texto;
    nova->horario = horario;
    nova->sala = sala;
    nova->marcada = false;

    if (inv->inicio == NULL || horario < inv->inicio->horario) {
        nova->prox = inv->inicio;
        inv->inicio = nova;
    } else {
        Pista *atual = inv->inicio;
        while (atual->prox != NULL && atual->prox->horario <= horario)
            atual = atual->prox;
        nova->prox = atual->prox;
        atual->prox = nova;
    }
    inv->quantidade++;
    return true;
}

Pista *inventario_buscar(const Inventario *inv, const char *texto) {
    for (Pista *p = inv->inicio; p != NULL; p = p->prox)
        if (strcmp(p->texto, texto) == 0) return p;
    return NULL;
}

Pista *inventario_buscar_horario(const Inventario *inv, int horario) {
    for (Pista *p = inv->inicio; p != NULL; p = p->prox)
        if (p->horario == horario) return p;
    return NULL;
}

bool inventario_tem_horario(const Inventario *inv, int horario) {
    return inventario_buscar_horario(inv, horario) != NULL;
}

Pista *inventario_na_posicao(const Inventario *inv, int indice) {
    int i = 0;
    for (Pista *p = inv->inicio; p != NULL; p = p->prox, i++)
        if (i == indice) return p;
    return NULL;
}

void inventario_alternar_marca(Inventario *inv, int indice) {
    Pista *p = inventario_na_posicao(inv, indice);
    if (p != NULL) p->marcada = !p->marcada;
}

int inventario_contar_marcadas(const Inventario *inv) {
    int n = 0;
    for (Pista *p = inv->inicio; p != NULL; p = p->prox)
        if (p->marcada) n++;
    return n;
}

void inventario_desmarcar_todas(Inventario *inv) {
    for (Pista *p = inv->inicio; p != NULL; p = p->prox) p->marcada = false;
}

void inventario_limpar(Inventario *inv) {
    while (inv->inicio != NULL) {
        Pista *aux = inv->inicio;
        inv->inicio = aux->prox;
        free(aux);
    }
    inv->quantidade = 0;
}
