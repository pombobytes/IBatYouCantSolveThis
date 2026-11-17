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

/* Paredes genéricas nas 4 bordas, com uma porta no meio de cada lateral.
   Usadas pelas salas sem imagem (ex.: o porão). */
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

    // porta da esquerda leva à sala anterior; a da direita, à próxima
    s->portaAnt  = (Porta){ (Rectangle){ 0, PORTA_Y, 16, PORTA_ALTURA },      (Vector2){ 40, PORTA_Y + 45 } };
    s->portaProx = (Porta){ (Rectangle){ W - 16, PORTA_Y, 16, PORTA_ALTURA }, (Vector2){ W - 80, PORTA_Y + 45 } };
}

static Sala *sala_nova(const char *nome, Color cor) {
    Sala *s = malloc(sizeof(Sala));
    if (s == NULL) {
        return NULL;   // malloc falhou (falta de memória)
    }
    s->nome = nome;
    s->corFundo = cor;
    s->fundo = (Texture2D){ 0 };   // sem imagem por padrão (ex.: porão)
    s->proxSala = s;
    s->antSala = s;
    s->qtdObstaculos = 0;
    s->qntdInterativo = 0;
    sala_criar_paredes(s);
    return s;
}

/* Carrega a imagem 640x360 da sala e troca as paredes genéricas pelas do desenho */
static void sala_usar_imagem(Sala *s, const char *caminho) {
    s->fundo = LoadTexture(caminho);
    if (s->fundo.id != 0) SetTextureFilter(s->fundo, TEXTURE_FILTER_POINT);   // pixel art sem borrar
    s->qtdObstaculos = 0;
}

/* ---------- Salão de Festas (assets/Salao.png) ----------
   Portas nas laterais (degraus de madeira). Bilhete com o mordomo,
   charada da campânula no bufê e o relógio de pêndulo da bomba. */
static void montar_salao(Sala *s) {
    sala_usar_imagem(s, "assets/Salao.png");
    // paredes do desenho, com as aberturas das portas laterais (y 285..432)
    sala_adicionar_obstaculo(s, (Rectangle){ 0, 0, 1280, 172 });      // parede de cima (janelas)
    sala_adicionar_obstaculo(s, (Rectangle){ 0, 640, 1280, 80 });     // balaústres de baixo
    sala_adicionar_obstaculo(s, (Rectangle){ 0, 172, 66, 113 });      // esquerda, acima da porta
    sala_adicionar_obstaculo(s, (Rectangle){ 0, 432, 66, 208 });      // esquerda, abaixo da porta
    sala_adicionar_obstaculo(s, (Rectangle){ 1214, 172, 66, 113 });   // direita, acima da porta
    sala_adicionar_obstaculo(s, (Rectangle){ 1214, 432, 66, 208 });   // direita, abaixo da porta
    s->portaAnt  = (Porta){ (Rectangle){ 0, 290, 20, 138 },    (Vector2){ 80, 345 } };
    s->portaProx = (Porta){ (Rectangle){ 1260, 290, 20, 138 }, (Vector2){ 1164, 345 } };

    // móveis
    Rectangle relogio = { 608, 150, 64, 62 };     // base do relógio de pêndulo
    Rectangle bufe    = { 770, 165, 210, 72 };
    Rectangle mesaSE  = { 135, 175, 165, 100 };   // mesas redondas com cadeiras
    Rectangle mesaSD  = { 1018, 180, 165, 105 };
    Rectangle mesaIE  = { 163, 505, 165, 110 };
    Rectangle mesaID  = { 963, 505, 165, 110 };
    Rectangle vasoIE  = { 66, 565, 50, 75 };
    Rectangle vasoID  = { 1165, 565, 50, 75 };
    Rectangle mordomo = { 704, 262, 32, 20 };     // pés do mordomo (sprite desenhado em jogo.c)
    sala_adicionar_obstaculo(s, relogio);
    sala_adicionar_obstaculo(s, bufe);
    sala_adicionar_obstaculo(s, mesaSE);
    sala_adicionar_obstaculo(s, mesaSD);
    sala_adicionar_obstaculo(s, mesaIE);
    sala_adicionar_obstaculo(s, mesaID);
    sala_adicionar_obstaculo(s, vasoIE);
    sala_adicionar_obstaculo(s, vasoID);
    sala_adicionar_obstaculo(s, mordomo);

    sala_adicionar_interativo(s, OBJ_BILHETE,   (Rectangle){ 690, 200, 60, 84 },  "Falar com o mordomo");
    sala_adicionar_interativo(s, OBJ_RELOGIO,   (Rectangle){ 600, 40, 80, 175 },  "Examinar o relógio");
    sala_adicionar_interativo(s, OBJ_CAMPANULA, (Rectangle){ 786, 150, 66, 90 },  "Levantar a campânula");
    sala_adicionar_interativo(s, OBJ_PRATOS,    (Rectangle){ 858, 160, 50, 80 },  "Examinar os pratos");
    sala_adicionar_interativo(s, OBJ_FLORES,    mesaSE,                           "Examinar o arranjo de flores");
    sala_adicionar_interativo(s, OBJ_VASO,      vasoIE,                           "Examinar o vaso");
}

/* ---------- Jardim de Inverno (assets/jardim.png) ----------
   Porta dupla em cima (vai para a próxima sala) e saída embaixo (volta). */
static void montar_jardim(Sala *s) {
    sala_usar_imagem(s, "assets/jardim.png");
    sala_adicionar_obstaculo(s, (Rectangle){ 0, 0, 1280, 190 });      // vidraças de cima
    sala_adicionar_obstaculo(s, (Rectangle){ 0, 692, 1280, 28 });     // borda de baixo
    sala_adicionar_obstaculo(s, (Rectangle){ 0, 190, 62, 502 });      // vidraça esquerda
    sala_adicionar_obstaculo(s, (Rectangle){ 1218, 190, 62, 502 });   // vidraça direita
    s->portaProx = (Porta){ (Rectangle){ 595, 186, 110, 14 }, (Vector2){ 632, 232 } };
    s->portaAnt  = (Porta){ (Rectangle){ 540, 660, 200, 32 }, (Vector2){ 632, 600 } };

    Rectangle fonte = { 505, 320, 295, 245 };
    sala_adicionar_obstaculo(s, fonte);
    sala_adicionar_obstaculo(s, (Rectangle){ 458, 345, 47, 160 });    // flores ao lado da fonte
    sala_adicionar_obstaculo(s, (Rectangle){ 800, 345, 47, 160 });
    sala_adicionar_obstaculo(s, (Rectangle){ 245, 215, 175, 105 });   // mesas
    sala_adicionar_obstaculo(s, (Rectangle){ 865, 215, 160, 110 });
    sala_adicionar_obstaculo(s, (Rectangle){ 935, 515, 150, 115 });
    sala_adicionar_obstaculo(s, (Rectangle){ 60, 390, 230, 72 });     // balcão do bar (em L)
    sala_adicionar_obstaculo(s, (Rectangle){ 60, 462, 100, 90 });
    sala_adicionar_obstaculo(s, (Rectangle){ 60, 550, 318, 90 });
    sala_adicionar_obstaculo(s, (Rectangle){ 90, 200, 95, 100 });     // plantas
    sala_adicionar_obstaculo(s, (Rectangle){ 485, 190, 50, 35 });
    sala_adicionar_obstaculo(s, (Rectangle){ 760, 190, 55, 35 });
    sala_adicionar_obstaculo(s, (Rectangle){ 1040, 190, 120, 100 });
    sala_adicionar_obstaculo(s, (Rectangle){ 1190, 270, 40, 90 });
    sala_adicionar_obstaculo(s, (Rectangle){ 80, 640, 110, 52 });
    sala_adicionar_obstaculo(s, (Rectangle){ 205, 640, 70, 52 });
    sala_adicionar_obstaculo(s, (Rectangle){ 445, 640, 50, 52 });
    sala_adicionar_obstaculo(s, (Rectangle){ 800, 640, 50, 52 });
    sala_adicionar_obstaculo(s, (Rectangle){ 1110, 600, 100, 92 });

    sala_adicionar_interativo(s, OBJ_GARCONS, (Rectangle){ 60, 390, 320, 250 }, "Conversar com os garçons do bar");
    sala_adicionar_interativo(s, OBJ_FONTE,   fonte,                              "Examinar a fonte");
}

/* ---------- Galeria de Arte (assets/galeria.png) ----------
   Porta dupla em cima (próxima sala) e tapete de saída embaixo (volta). */
static void montar_galeria(Sala *s) {
    sala_usar_imagem(s, "assets/galeria.png");
    sala_adicionar_obstaculo(s, (Rectangle){ 0, 0, 1280, 195 });      // parede dos quadros
    sala_adicionar_obstaculo(s, (Rectangle){ 0, 640, 595, 80 });      // parede de baixo, com a saída no meio
    sala_adicionar_obstaculo(s, (Rectangle){ 715, 640, 565, 80 });
    sala_adicionar_obstaculo(s, (Rectangle){ 0, 195, 12, 445 });      // laterais
    sala_adicionar_obstaculo(s, (Rectangle){ 1268, 195, 12, 445 });
    s->portaProx = (Porta){ (Rectangle){ 598, 192, 114, 14 }, (Vector2){ 637, 232 } };
    s->portaAnt  = (Porta){ (Rectangle){ 598, 690, 114, 30 }, (Vector2){ 637, 590 } };

    Rectangle divisoriaE = { 155, 280, 378, 180 };   // painéis com quadros e cordões
    Rectangle divisoriaD = { 770, 280, 372, 180 };
    sala_adicionar_obstaculo(s, divisoriaE);
    sala_adicionar_obstaculo(s, divisoriaD);
    sala_adicionar_obstaculo(s, (Rectangle){ 0, 195, 50, 60 });       // armário
    sala_adicionar_obstaculo(s, (Rectangle){ 390, 175, 55, 60 });     // bustos
    sala_adicionar_obstaculo(s, (Rectangle){ 865, 175, 55, 60 });
    sala_adicionar_obstaculo(s, (Rectangle){ 50, 340, 60, 115 });
    sala_adicionar_obstaculo(s, (Rectangle){ 1188, 340, 60, 115 });
    sala_adicionar_obstaculo(s, (Rectangle){ 390, 520, 55, 120 });
    sala_adicionar_obstaculo(s, (Rectangle){ 843, 520, 55, 120 });
    sala_adicionar_obstaculo(s, (Rectangle){ 520, 190, 55, 22 });     // vasos
    sala_adicionar_obstaculo(s, (Rectangle){ 745, 190, 55, 22 });
    sala_adicionar_obstaculo(s, (Rectangle){ 1228, 195, 40, 35 });
    sala_adicionar_obstaculo(s, (Rectangle){ 5, 480, 45, 160 });
    sala_adicionar_obstaculo(s, (Rectangle){ 1225, 610, 43, 30 });

    sala_adicionar_interativo(s, OBJ_QUADROS, divisoriaE, "Examinar os quadros do leilão");
    sala_adicionar_interativo(s, OBJ_QUADROS, divisoriaD, "Examinar os quadros do leilão");
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

    montar_salao(salao);
    montar_jardim(jardim);
    montar_galeria(galeria);

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
        if (atual->fundo.id != 0) UnloadTexture(atual->fundo);   // imagem da sala
        free(atual);
        atual = proxima;
    }
}
