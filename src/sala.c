#include <stdlib.h>
#include "tela.h"
#include "sala.h"

bool sala_adicionar_obstaculo(Sala *s, Rectangle r) {
    if (s->qtdObstaculos >= MAX_OBSTACULOS) return false;
    s->obstaculos[s->qtdObstaculos++] = r;
    return true;
}

bool sala_adicionar_interativo(Sala *s, ObjetoId id, Rectangle area, const char *rotulo) {
    if (s->qntdInterativo >= MAX_INTERATIVOS) return false;
    s->interativos[s->qntdInterativo++] = (Interativo){ id, area, rotulo };
    return true;
}

/* Paredes nas 4 bordas, com uma abertura (porta) no meio das laterais.
   Como são obstáculos, a colisão que já existe impede o Bruno de sair da
   sala por qualquer lugar que não seja a porta. */
static void sala_criar_paredes(Sala *s) {
    const float T = PAREDE_ESPESSURA;
    const float W = TELA_LARGURA, H = TELA_ALTURA;
    const float fimPorta = PORTA_Y + PORTA_ALTURA;

    sala_adicionar_obstaculo(s, (Rectangle){ 0, 0, W, T });                          // cima
    sala_adicionar_obstaculo(s, (Rectangle){ 0, H - T, W, T });                      // baixo
    sala_adicionar_obstaculo(s, (Rectangle){ 0, T, T, PORTA_Y - T });                // esquerda, acima da porta
    sala_adicionar_obstaculo(s, (Rectangle){ 0, fimPorta, T, H - T - fimPorta });    // esquerda, abaixo da porta
    sala_adicionar_obstaculo(s, (Rectangle){ W - T, T, T, PORTA_Y - T });            // direita, acima da porta
    sala_adicionar_obstaculo(s, (Rectangle){ W - T, fimPorta, T, H - T - fimPorta });// direita, abaixo da porta
}

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
    s->qntdInterativo = 0;
    sala_criar_paredes(s);   // obstáculos [0 .. NUM_PAREDES-1]
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

    //Salão de Festas: bilhete, charada e o relógio da bomba 
    Rectangle mesa    = { 480, 170, 240, 90 };
    Rectangle piano   = { 1020, 110, 170, 100 };
    Rectangle relogio = { 90, 60, 50, 90 };
    sala_adicionar_obstaculo(salao, mesa);
    sala_adicionar_obstaculo(salao, piano);
    sala_adicionar_obstaculo(salao, relogio);
    sala_adicionar_interativo(salao, OBJ_BILHETE, (Rectangle){ 585, 268, 30, 20 }, "Ler o bilhete");
    sala_adicionar_interativo(salao, OBJ_RELOGIO, relogio, "Examinar o relógio");
    sala_adicionar_interativo(salao, OBJ_PIANO, piano, "Examinar o piano");
    sala_adicionar_interativo(salao, OBJ_TACA, (Rectangle){ 300, 500, 24, 24 }, "Examinar a taça");
    sala_adicionar_interativo(salao, OBJ_CARDAPIO, (Rectangle){ 880, 560, 32, 24 }, "Ler o cardápio");
    sala_adicionar_interativo(salao, OBJ_REPERTORIO, (Rectangle){ 160, 610, 32, 24 }, "Ler o repertório da banda");

    //Jardim de Inverno: garçons e a fonte 
    Rectangle fonte   = { 560, 250, 160, 160 };
    Rectangle garcons = { 260, 110, 120, 60 };
    sala_adicionar_obstaculo(jardim, fonte);
    sala_adicionar_obstaculo(jardim, garcons);
    sala_adicionar_obstaculo(jardim, (Rectangle){ 1000, 500, 80, 80 });   // vaso de plantas
    sala_adicionar_interativo(jardim, OBJ_GARCONS, garcons, "Conversar com os garçons");
    sala_adicionar_interativo(jardim, OBJ_FONTE, fonte, "Examinar a fonte");

    // Galeria de Arte: os quadros do leilão 
    sala_adicionar_obstaculo(galeria, (Rectangle){ 560, 420, 160, 50 });  // banco
    sala_adicionar_interativo(galeria, OBJ_QUADROS, (Rectangle){ 340, 30, 600, 30 }, "Examinar os quadros do leilão");

    return salao;
}

Sala *sala_ir_direita(Sala *atual) {
    return atual->proxSala;
}

Sala *sala_ir_esquerda(Sala *atual) {
    return atual->antSala;
}

Sala *sala_inserir_depois(Sala *ref, const char *nome, Color cor) {
    Sala *nova = sala_nova(nome, cor);
    if (nova == NULL) return NULL;

    nova->proxSala = ref->proxSala;
    nova->antSala = ref;
    ref->proxSala->antSala = nova;
    ref->proxSala = nova;

    return nova;
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
