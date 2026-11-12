#include <stdlib.h>
#include "sala.h"

static Sala *sala_nova(const char *nome, Color cor) {
    Sala *s = malloc(sizeof(Sala));
    if (s == NULL) {
        return NULL;   // malloc falhou (falta de memória)
    }
    s->nome = nome;
    s->corFundo = cor; 
    s->proxSala = s;
    s->antSala = s;
    s->qtdObstaculos = 0;
    return s;
}

Sala *salas_criar(void) {
    Sala *salao   = sala_nova("Salão de Festas", (Color){ 100, 20, 35, 255 });
    Sala *jardim  = sala_nova("Jardim de Inverno",(Color){ 30, 65, 40, 255 });
    Sala *galeria = sala_nova("Galeria de Arte", (Color){ 25, 35, 70, 255 });

    if(salao == NULL || jardim == NULL || galeria == NULL){
        free(salao);
        free(jardim);
        free(galeria);
        return NULL;
    }

    salao->proxSala = jardim;
    salao->antSala = galeria;
    jardim->antSala = salao;
    jardim->proxSala = galeria;
    galeria->antSala = jardim;
    galeria->proxSala = salao;

    salao->obstaculos[0] = (Rectangle){500, 300, 200, 100};
    salao->qtdObstaculos = 1;
    
    return salao;
}

Sala *sala_ir_direita(Sala *atual) {
    return atual->proxSala;
}

Sala *sala_ir_esquerda(Sala *atual) {
    return atual->antSala;
}

void salas_liberar(Sala *inicio) {
    if (inicio == NULL) return;

    inicio->antSala->proxSala = NULL;
    Sala *atual = inicio;
    while(atual != NULL){
        Sala *proxima = atual->proxSala;
        free(atual);
        atual = proxima;
    }
}