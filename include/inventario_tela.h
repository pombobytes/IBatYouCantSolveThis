#ifndef INVENTARIO_TELA_H
#define INVENTARIO_TELA_H
#include <stdbool.h>
#include "jogador.h"

// Tela da linha do tempo (tecla I). Na acusação, podeMarcar = true. 
typedef struct {
    bool aberta;
    bool podeMarcar;
    int  selecionada;
} TelaInventario;

void inventario_tela_abrir(TelaInventario *t, bool podeMarcar);
bool inventario_tela_atualizar(TelaInventario *t, Inventario *inv); // true ao fechar/confirmar 
void inventario_tela_desenhar(const TelaInventario *t, const Inventario *inv);

#endif
