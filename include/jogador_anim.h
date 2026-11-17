#ifndef JOGADOR_ANIM_H
#define JOGADOR_ANIM_H

#include "raylib.h"

void jogador_anim_iniciar(Vector2 posicao);                 // depois do InitWindow
void jogador_anim_atualizar(Vector2 posicao, float dt);     // 1x por frame
void jogador_anim_desenhar(Vector2 posicao, float larguraHitbox, float alturaHitbox);
void jogador_anim_finalizar(void);

#endif