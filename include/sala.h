#ifndef SALA_H
#define SALA_H
#include <stdbool.h>
#include "raylib.h"

#define MAX_OBSTACULOS  32
#define MAX_INTERATIVOS 10

/* Paredes e portas genéricas (resolução fixa 1280x720, ver tela.h).
   Uma sala sem imagem de fundo (ex.: o porão) nasce com paredes nas bordas e
   uma porta no meio de cada lateral. Salas com imagem trocam essas paredes e
   portas pelas que combinam com o desenho (ver salas_criar). */
#define PAREDE_ESPESSURA 24
#define PORTA_Y          300   // topo da abertura das portas laterais genéricas
#define PORTA_ALTURA     120

typedef enum {
    OBJ_BILHETE,      // o mordomo entrega o bilhete
    OBJ_RELOGIO,      // relógio do salão: onde está a bomba (acusação + fios)
    // puzzle do salão (charada do bilhete)
    OBJ_CAMPANULA,    // resposta da charada: a campânula de prata do bufê
    OBJ_FLORES,
    OBJ_PRATOS,
    OBJ_VASO,
    // puzzle do jardim (quem está mentindo)
    OBJ_GARCONS,
    OBJ_FONTE,        // também abre o porão depois da galeria
    // puzzle da galeria (insertion sort)
    OBJ_QUADROS,
    // porão secreto
    OBJ_OTAVIO,
    // os colegas acrescentam aqui os ids dos objetos deles
} ObjetoId;

typedef struct {
    ObjetoId id;
    Rectangle area;
    const char *rotulo;   // texto que aparece no "[E] ..."
} Interativo;

/* Uma porta: encostar o Bruno no gatilho troca de sala.
   'chegada' é onde o Bruno aparece quando ENTRA nesta sala por esta porta. */
typedef struct {
    Rectangle gatilho;
    Vector2 chegada;
} Porta;

typedef struct Sala{
    const char * nome;
    Color corFundo;           // usada quando não há imagem
    Texture2D fundo;          // imagem 640x360 da sala (id 0 = sem imagem)
    Rectangle obstaculos[MAX_OBSTACULOS];
    int qtdObstaculos;
    struct Sala *proxSala;
    struct Sala *antSala;
    Porta portaProx;          // leva para proxSala
    Porta portaAnt;           // leva para antSala
    Interativo interativos[MAX_INTERATIVOS];
    int qntdInterativo;
} Sala;

Sala *salas_criar(void);           // monta o anel com as 3 salas e devolve a primeira
void  salas_liberar(Sala *inicio); // libera todos os nós (e as imagens)
Sala *sala_ir_direita(Sala *atual);
Sala *sala_ir_esquerda(Sala *atual);
Sala *sala_inserir_depois(Sala *ref, const char *nome, Color cor);

/* Acrescentam um obstáculo / objeto interativo na sala.
   Devolvem false se o vetor já estiver cheio. */
bool  sala_adicionar_obstaculo(Sala *s, Rectangle r);
bool  sala_adicionar_interativo(Sala *s, ObjetoId id, Rectangle area, const char *rotulo);
#endif
