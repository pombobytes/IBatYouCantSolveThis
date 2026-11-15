#ifndef JOGADOR_H
#define JOGADOR_H

#include <stdbool.h>
#include "raylib.h"
#include "sala.h"

/* ---------- Inventário (lista encadeada simples ordenada por horário) ---------- */

typedef struct Pista {
    const char *texto;      // a fala / pista (string literal)
    int horario;            // minutos desde 00:00 (22h40 = 1360)
    const char *sala;       // nome da sala onde foi coletada
    bool marcada;           // marcada na acusação
    struct Pista *prox;
} Pista;

typedef struct {
    Pista *inicio;          // pista de horário mais cedo
    int quantidade;
} Inventario;

/* ---------- Jogador ---------- */

typedef struct {
    Vector2 posicao;            // canto superior esquerdo da hitbox
    Sala *salaAtual;
    Inventario inventario;      // criado junto com o jogador
} Jogador;

void   jogador_iniciar(Jogador *j, Sala *salaInicial, Vector2 posicao);
void   jogador_finalizar(Jogador *j);
bool   jogador_coletar_fala(Jogador *j, const char *texto, int horario);

void   inventario_iniciar(Inventario *inv);
bool   inventario_inserir(Inventario *inv, const char *texto, int horario, const char *sala);
Pista *inventario_buscar(const Inventario *inv, const char *texto);
void   inventario_limpar(Inventario *inv);

#endif