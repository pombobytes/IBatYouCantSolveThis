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
    s->portaAnt  = (Porta){ (Rectangle){ 0, PORTA_Y, 16, PORTA_ALTURA },      (Vector2){ 40, PORTA_Y + 45 }, false };
    s->portaProx = (Porta){ (Rectangle){ W - 16, PORTA_Y, 16, PORTA_ALTURA }, (Vector2){ W - 80, PORTA_Y + 45 }, false };
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

/* Carrega a imagem da sala (640x360 ou 1920x1080) e troca as paredes genéricas pelas do desenho */
static void sala_usar_imagem(Sala *s, const char *caminho) {
    s->fundo = LoadTexture(caminho);
    // 640x360: pixel art ampliada 3x, sem borrar. 1920x1080: já está no tamanho da tela.
    if (s->fundo.id != 0)
        SetTextureFilter(s->fundo, s->fundo.width <= 640 ? TEXTURE_FILTER_POINT : TEXTURE_FILTER_BILINEAR);
    s->qtdObstaculos = 0;
}

/* Salão de Festas
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
    s->portaAnt  = (Porta){ (Rectangle){ 0, 290, 20, 138 },    (Vector2){ 80, 345 }, false };
    s->portaProx = (Porta){ (Rectangle){ 1260, 290, 20, 138 }, (Vector2){ 1164, 345 }, false };

    // móveis
    Rectangle relogio = { 608, 150, 64, 62 };     // base do relógio de pêndulo
    Rectangle bufe    = { 770, 165, 210, 72 };
    Rectangle mesaSE  = { 135, 215, 165, 55 };   // mesas redondas: só a base (com as cadeiras)
    Rectangle mesaSD  = { 1018, 220, 165, 60 };
    Rectangle mesaIE  = { 163, 545, 165, 65 };
    Rectangle mesaID  = { 963, 545, 165, 65 };
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

/* Jardim de Inverno 
   Porta dupla em cima (vai para a próxima sala) e passagem embaixo, entre os vasos (volta).
   Garçons em pé à esquerda; convidados nos bancos (desenhados em jogo.c). */
static void montar_jardim(Sala *s) {
    sala_usar_imagem(s, "assets/jardim.png");
    sala_adicionar_obstaculo(s, (Rectangle){ 0, 0, 1280, 205 });       // vidraças e porta de cima
    sala_adicionar_obstaculo(s, (Rectangle){ 0, 205, 55, 515 });       // parede esquerda (trepadeiras)
    sala_adicionar_obstaculo(s, (Rectangle){ 1225, 205, 55, 515 });    // parede direita
    sala_adicionar_obstaculo(s, (Rectangle){ 0, 672, 535, 48 });       // parede de baixo, com a passagem no meio
    sala_adicionar_obstaculo(s, (Rectangle){ 745, 672, 535, 48 });
    s->portaProx = (Porta){ (Rectangle){ 595, 200, 90, 14 },  (Vector2){ 622, 232 }, false };
    s->portaAnt  = (Porta){ (Rectangle){ 540, 690, 200, 30 }, (Vector2){ 622, 620 }, false };

    Rectangle fonte = { 525, 318, 232, 186 };
    sala_adicionar_obstaculo(s, fonte);
    sala_adicionar_obstaculo(s, (Rectangle){ 95, 185, 190, 90 });      // canteiros de cima
    sala_adicionar_obstaculo(s, (Rectangle){ 1000, 185, 190, 90 });
    sala_adicionar_obstaculo(s, (Rectangle){ 298, 215, 165, 50 });     // bancos de cima (só a base)
    sala_adicionar_obstaculo(s, (Rectangle){ 822, 215, 165, 50 });
    sala_adicionar_obstaculo(s, (Rectangle){ 475, 195, 70, 50 });      // vasos ao lado da porta
    sala_adicionar_obstaculo(s, (Rectangle){ 745, 195, 65, 50 });
    sala_adicionar_obstaculo(s, (Rectangle){ 65, 380, 70, 85 });       // pedestais com flores
    sala_adicionar_obstaculo(s, (Rectangle){ 1150, 380, 70, 85 });
    sala_adicionar_obstaculo(s, (Rectangle){ 40, 560, 248, 112 });     // canteiros de baixo
    sala_adicionar_obstaculo(s, (Rectangle){ 993, 560, 248, 112 });
    sala_adicionar_obstaculo(s, (Rectangle){ 295, 625, 165, 50 });     // bancos de baixo (só a base)
    sala_adicionar_obstaculo(s, (Rectangle){ 822, 625, 165, 50 });
    sala_adicionar_obstaculo(s, (Rectangle){ 190, 415, 110, 45 });     // pés dos garçons

    sala_adicionar_interativo(s, OBJ_GARCONS, (Rectangle){ 180, 340, 130, 120 }, "Conversar com os garçons");
    sala_adicionar_interativo(s, OBJ_FONTE,   fonte,                               "Examinar a fonte");
}

/* Galeria de Arte
   Os 6 quadros do leilão ficam na parede de cima. O desenho não tem porta,
   então as passagens são nas laterais e o jogo marca elas na tela. */
static void montar_galeria(Sala *s) {
    sala_usar_imagem(s, "assets/galeria.png");
    sala_adicionar_obstaculo(s, (Rectangle){ 0, 0, 1280, 250 });       // parede dos quadros
    sala_adicionar_obstaculo(s, (Rectangle){ 0, 668, 1280, 52 });      // parede de baixo
    sala_adicionar_obstaculo(s, (Rectangle){ 0, 250, 36, 180 });       // laterais, com a passagem no meio (y 430..550)
    sala_adicionar_obstaculo(s, (Rectangle){ 0, 550, 36, 118 });
    sala_adicionar_obstaculo(s, (Rectangle){ 1244, 250, 36, 180 });
    sala_adicionar_obstaculo(s, (Rectangle){ 1244, 550, 36, 118 });
    s->portaAnt  = (Porta){ (Rectangle){ 0, 430, 20, 120 },    (Vector2){ 50, 476 },   true };
    s->portaProx = (Porta){ (Rectangle){ 1260, 430, 20, 120 }, (Vector2){ 1194, 476 }, true };

    sala_adicionar_obstaculo(s, (Rectangle){ 30, 245, 150, 50 });      // bustos com cordões
    sala_adicionar_obstaculo(s, (Rectangle){ 1095, 245, 155, 50 });
    sala_adicionar_obstaculo(s, (Rectangle){ 36, 385, 60, 40 });       // vasos (só a base)
    sala_adicionar_obstaculo(s, (Rectangle){ 1185, 385, 60, 40 });
    sala_adicionar_obstaculo(s, (Rectangle){ 36, 625, 60, 43 });
    sala_adicionar_obstaculo(s, (Rectangle){ 1185, 625, 60, 43 });
    sala_adicionar_obstaculo(s, (Rectangle){ 242, 600, 220, 50 });     // bancos (só a base)
    sala_adicionar_obstaculo(s, (Rectangle){ 818, 600, 220, 50 });

    sala_adicionar_interativo(s, OBJ_QUADROS, (Rectangle){ 215, 50, 850, 205 }, "Examinar os quadros do leilão");
}

/*Porão secreto
   Escada no canto de cima à esquerda (volta ao jardim) e corredor embaixo (segue o anel).
   O Otávio é colocado por jogo_abrir_porao. */
void sala_montar_porao(Sala *s) {
    sala_usar_imagem(s, "assets/porao.png");
    if (s->fundo.id == 0) return;   // sem imagem: fica com as paredes genéricas
    sala_adicionar_obstaculo(s, (Rectangle){ 0, 0, 55, 720 });         // parede esquerda
    sala_adicionar_obstaculo(s, (Rectangle){ 1228, 0, 52, 720 });      // parede direita
    sala_adicionar_obstaculo(s, (Rectangle){ 55, 0, 120, 200 });       // topo da escada
    sala_adicionar_obstaculo(s, (Rectangle){ 175, 0, 40, 255 });       // parede ao lado da escada
    sala_adicionar_obstaculo(s, (Rectangle){ 215, 0, 1013, 248 });     // estante, mesa, armário e lençóis de cima
    sala_adicionar_obstaculo(s, (Rectangle){ 222, 300, 70, 140 });      // caixotes de cima
    sala_adicionar_obstaculo(s, (Rectangle){ 1160, 285, 60, 60 });     // vaso da direita
    sala_adicionar_obstaculo(s, (Rectangle){ 220, 290, 75, 150 });     // caixotes da esquerda
    sala_adicionar_obstaculo(s, (Rectangle){ 55, 345, 160, 295 });     // lençol, barril e caixotes da esquerda
    sala_adicionar_obstaculo(s, (Rectangle){ 235, 625, 65, 35 });      // vaso de baixo
    sala_adicionar_obstaculo(s, (Rectangle){ 1088, 370, 140, 270 });   // lençol, caixotes e barril da direita
    sala_adicionar_obstaculo(s, (Rectangle){ 0, 650, 600, 70 });       // parede de baixo, com o corredor no meio
    sala_adicionar_obstaculo(s, (Rectangle){ 1000, 650, 280, 70 });
    s->portaAnt  = (Porta){ (Rectangle){ 60, 196, 115, 14 },  (Vector2){ 98, 296 }, false };
    s->portaProx = (Porta){ (Rectangle){ 600, 690, 400, 30 }, (Vector2){ 782, 600 }, true };
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
