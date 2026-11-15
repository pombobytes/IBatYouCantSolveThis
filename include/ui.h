#ifndef UI_H
#define UI_H
#include <stdbool.h>

#define MAX_FALAS   16
#define MAX_OPCOES  8

/* ---- Caixa de dialogo: uma FILA de falas (E avanca = desenfileira) ---- */
typedef struct {
    const char *falas[MAX_FALAS];
    const char *quemFala;   /* nome mostrado no topo da caixa (pode ser NULL) */
    int inicio, fim;        /* inicio = proxima fala a mostrar */
    bool aberto;
} Dialogo;

void dialogo_abrir(Dialogo *d, const char *quemFala, const char *falas[], int n);
bool dialogo_atualizar(Dialogo *d);   /* true no frame em que a ultima fala e fechada */
void dialogo_desenhar(const Dialogo *d);

/* ---- Escolher entre N opcoes ---- */
typedef struct {
    const char *titulo;
    const char *opcoes[MAX_OPCOES];
    int n, selecionada;
    bool aberta;
} Escolha;

void escolha_abrir(Escolha *e, const char *titulo, const char *opcoes[], int n);
int  escolha_atualizar(Escolha *e);   /* indice escolhido (Enter) ou -1 enquanto nada */
void escolha_desenhar(const Escolha *e);

/* ---- Aviso rapido ("-15 min", "Pista adicionada") ---- */
void aviso_mostrar(const char *texto, float segundos);
void aviso_atualizar(float dt);
void aviso_desenhar(void);

#endif
