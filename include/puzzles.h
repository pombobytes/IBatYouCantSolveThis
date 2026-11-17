#ifndef PUZZLES_H
#define PUZZLES_H
#include <stdbool.h>
#include "ui.h"

/* Resultado de um frame de puzzle */
typedef enum { PZ_RODANDO, PZ_ACERTOU, PZ_ERROU, PZ_SAIU } ResultadoPuzzle;

/* Qual puzzle está aberto no estado PUZZLE */
typedef enum { PUZZLE_NENHUM, PUZZLE_JARDIM, PUZZLE_GALERIA } PuzzleId;

/* JARDIM: quem está mentindo?
   Os depoimentos dos 4 garçons são uma FILA de falas (Dialogo do ui.h);
   depois o jogador escolhe o mentiroso (Escolha do ui.h).
   Exatamente um mente: Edu.                                         */
typedef enum { JARDIM_DEPOIMENTOS, JARDIM_ESCOLHA } EtapaJardim;

typedef struct {
    EtapaJardim etapa;
    Dialogo dialogo;
    Escolha escolha;
} PuzzleJardim;

void            jardim_iniciar(PuzzleJardim *p);
ResultadoPuzzle jardim_atualizar(PuzzleJardim *p);
void            jardim_desenhar(const PuzzleJardim *p);

/* GALERIA: quadros do leilão (insertion sort) 
   Os quadros estão fora de ordem num vetor. O jogador PEGA um quadro e o
   INSERE em outra posição; os outros deslizam para abrir espaço — exatamente
   o passo do insertion sort (quadros_mover). O jogo confere a resposta
   ordenando uma cópia do vetor com insertion sort (quadros_insertion_sort). */
#define NUM_QUADROS 6

typedef struct {
    const char *titulo;
    int ano;
} Quadro;

typedef struct {
    Quadro quadros[NUM_QUADROS];
    int cursor;            // posição selecionada
    int pego;              // índice do quadro pego, ou -1
    int movimentos;
    bool destacarAntigo;   // dica do jardim: o quadro mais antigo aparece destacado
} PuzzleGaleria;

void            galeria_iniciar(PuzzleGaleria *p, bool destacarAntigo);
ResultadoPuzzle galeria_atualizar(PuzzleGaleria *p);
void            galeria_desenhar(const PuzzleGaleria *p);

/* Algoritmos usados pelo puzzle (públicos para facilitar testes/explicação) */
void quadros_insertion_sort(Quadro v[], int n);           // ordena por ano (crescente)
void quadros_mover(Quadro v[], int n, int de, int para);  // tira de 'de' e insere em 'para'
bool quadros_em_ordem(const Quadro v[], int n);           // compara com uma cópia ordenada

#endif
