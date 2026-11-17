#ifndef TELA_H
#define TELA_H

/* Resolução interna fixa do jogo.
   Tudo é desenhado neste tamanho e depois ampliado para caber na janela
   (ver main.c), então posições de paredes, portas, objetos e sprites são
   sempre as mesmas, em qualquer monitor. Use estas constantes no lugar de
   GetScreenWidth()/GetScreenHeight(). */
#define TELA_LARGURA 1280
#define TELA_ALTURA  720

#endif
