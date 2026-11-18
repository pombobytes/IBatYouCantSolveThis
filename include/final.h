#ifndef FINAL_H
#define FINAL_H
#include <stdbool.h>
#include "raylib.h"
#include "jogador.h"
#include "ui.h"
#include "inventario_tela.h"

typedef enum { SUSPEITO_HELENA, SUSPEITO_OTAVIO, SUSPEITO_ALMEIDA, TOTAL_SUSPEITOS } Suspeito;
typedef enum { RESULTADO_NENHUM, RESULTADO_MUITO_BOM, RESULTADO_BOM,RESULTADO_MEDIO, RESULTADO_RUIM } ResultadoFinal;

// Configuração do mistério (mude aqui se o grupo decidir outra coisa) As pistas são identificadas pelo HORÁRIO, que é único na linha do tempo.         
#define CULPADO      SUSPEITO_OTAVIO
#define NUM_PROVAS   3
#define NUM_FIOS     4
#define ERROS_MAX    2
extern const int PROVAS[NUM_PROVAS];       // horários das pistas que provam a culpa 
extern const int ORDEM_FIOS[NUM_FIOS];     // horários na ordem de corte 

//Acusação 
typedef enum { ACUS_ESCOLHER_SUSPEITO, ACUS_MARCAR_PISTAS, ACUS_CONCLUIDA } EtapaAcusacao;

typedef struct {
    EtapaAcusacao  etapa;
    Escolha        escolha;
    TelaInventario tela;
    int  suspeito;
    bool acertou;
} Acusacao;

void acusacao_iniciar(Acusacao *a, Inventario *inv);
bool acusacao_atualizar(Acusacao *a, Inventario *inv);   /* true quando terminou */
void acusacao_desenhar(const Acusacao *a, const Inventario *inv);
bool acusacao_verificar(int suspeito, const Inventario *inv);

//Fios da bomba
typedef struct {
    const char *rotulo;
    int   horario;      /* a ordem de corte segue o horário desta pista */
    Color cor;
    bool  cortado;
} Fio;

typedef struct {
    Fio  fios[NUM_FIOS];
    int  selecionado;
    int  proximo;
    int  erros;
    bool desarmada, explodiu;
} Bomba;

void bomba_iniciar(Bomba *b);
bool bomba_atualizar(Bomba *b);   /* true quando desarmou ou explodiu */
void bomba_desenhar(const Bomba *b);

//Finais 
ResultadoFinal final_decidir(bool capturouNoPorao, bool acusouCerto, bool desarmou, bool deuMeiaNoite);
void final_desenhar(ResultadoFinal f);

#endif
