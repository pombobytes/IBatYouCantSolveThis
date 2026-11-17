#include <stdlib.h>
#include "jogo.h"

#define DISTANCIA_INTERACAO 20

#define QTD(v) ((int)(sizeof(v) / sizeof((v)[0])))   // quantidade de itens de um vetor

static const char *titulo = "I Bat You Can't Solve It";
static const char *subtitulo = "Pressione ENTER para começar";
static const char *controles = "WASD/Setas: andar   E: interagir   I: linha do tempo   F11: tela cheia";
static const int tamanhoFonte = 40;
static const int tamanhoFonteSub = 20;

/* ---------------- recursos (imagens) ---------------- */
static Texture2D fundoMenu;
static bool fundoMenuOk = false;   // se a imagem não carregar, o menu usa só texto

void jogo_carregar_recursos(void) {
    fundoMenu = LoadTexture("assets/menu.png");
    fundoMenuOk = (fundoMenu.id != 0);
    if (fundoMenuOk) SetTextureFilter(fundoMenu, TEXTURE_FILTER_BILINEAR);
}

void jogo_liberar_recursos(void) {
    if (fundoMenuOk) UnloadTexture(fundoMenu);
    fundoMenuOk = false;
}

/* Botões do menu, por cima dos botões desenhados em assets/menu.png
   (coordenadas na resolução fixa 1280x720) */
#define QTD_OPCOES_MENU 3
static const Rectangle BOTOES_MENU[QTD_OPCOES_MENU] = {
    { 479, 370, 331, 68 },   // COMEÇAR
    { 488, 454, 314, 61 },   // CONTROLES
    { 488, 532, 314, 60 },   // CRÉDITOS
};
static const char *ROTULOS_MENU[QTD_OPCOES_MENU] = { "COMEÇAR", "CONTROLES", "CRÉDITOS" };

// Posição inicial do Bruno no salão (longe da mesa e das portas) 
#define SPAWN_X 624.0f
#define SPAWN_Y 470.0f


void jogo_iniciar(Jogo *jogo)
{
    jogo->estado = MENU;
    jogo->telaMenu = TELA_MENU_PRINCIPAL;
    jogo->opcaoMenu = 0;
    jogo->tempoRestante = TEMPO_TOTAL;
    jogo->salas = salas_criar();
    jogador_iniciar(&jogo->jogador, jogo->salas, (Vector2){ SPAWN_X, SPAWN_Y });
    jogo->interativoProximo = -1;
    jogo->dialogo.aberto = false;
    jogo->poraoAberto = false;
    jogo->telaInventario.aberta = false;

    jogo->puzzleAtual = PUZZLE_NENHUM;
    jogo->leuBilhete = false;
    jogo->salaoResolvido = false;
    jogo->jardimResolvido = false;
    jogo->galeriaResolvida = false;
    jogo->jardimPenalizado = false;
    jogo->galeriaPenalizada = false;
    jogo->terminarAposDialogo = false;
    jogo->resultadoPendente = RESULTADO_NENHUM;

    jogo->relogioLiberado = false;
    jogo->acusouCerto = false;
    jogo->capturouNoPorao = false;
    jogo->resultado = RESULTADO_NENHUM;
}

// Termina a partida: escolhe o final e vai para a tela FIM
static void jogo_terminar(Jogo *jogo, ResultadoFinal resultado) {
    jogo->resultado = resultado;
    jogo->estado = FIM;
}

//auxiliares usados pelas interações 

// Abre uma sequência de falas (fila do ui.h) e vai para o estado DIALOGO
static void abrir_dialogo(Jogo *jogo, const char *quem, const char *falas[], int n) {
    dialogo_abrir(&jogo->dialogo, quem, falas, n);
    jogo->estado = DIALOGO;
}

// Coloca uma pista na linha do tempo; avisa só se ela for nova
static void coletar_pista(Jogo *jogo, const char *texto, int horario) {
    if (jogador_coletar_fala(&jogo->jogador, texto, horario)) {
        aviso_mostrar("Pista adicionada à linha do tempo", 2.0f);
    }
}

// Penalidade de tempo com aviso na tela
static void penalizar(Jogo *jogo, int minutos, const char *motivo) {
    jogo_penalizar(jogo, minutos);
    aviso_mostrar(TextFormat("-%d min: %s", minutos, motivo), 3.0f);
}

static void menu_escolher(Jogo *jogo, int opcao)
{
    switch (opcao) {
        case 0: jogo->estado = EXPLORANDO; break;                 // COMEÇAR
        case 1: jogo->telaMenu = TELA_MENU_CONTROLES; break;      // CONTROLES
        case 2: jogo->telaMenu = TELA_MENU_CREDITOS; break;       // CRÉDITOS
        default: break;
    }
}

static void atualizar_menu(Jogo *jogo)
{
    // telas de controles e créditos: qualquer uma destas teclas (ou clique) volta
    if (jogo->telaMenu != TELA_MENU_PRINCIPAL) {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E) ||
            IsKeyPressed(KEY_BACKSPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            jogo->telaMenu = TELA_MENU_PRINCIPAL;
        }
        return;
    }

    // teclado: W/S ou setas trocam o botão (dá a volta), ENTER/E escolhe
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
        jogo->opcaoMenu = (jogo->opcaoMenu + 1) % QTD_OPCOES_MENU;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
        jogo->opcaoMenu = (jogo->opcaoMenu - 1 + QTD_OPCOES_MENU) % QTD_OPCOES_MENU;
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_E)) {
        menu_escolher(jogo, jogo->opcaoMenu);
        return;
    }

    // mouse: passar por cima seleciona, clicar escolhe
    // (main.c ajusta o mouse para a resolução fixa com SetMouseOffset/SetMouseScale)
    Vector2 mouse = GetMousePosition();
    Vector2 delta = GetMouseDelta();
    for (int i = 0; i < QTD_OPCOES_MENU; i++) {
        if (CheckCollisionPointRec(mouse, BOTOES_MENU[i])) {
            if (delta.x != 0 || delta.y != 0) jogo->opcaoMenu = i;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                jogo->opcaoMenu = i;
                menu_escolher(jogo, i);
            }
        }
    }
}

//colisao
static bool colide_com_sala(const Jogo *jogo) {
    Rectangle bruno = {jogo->jogador.posicao.x,jogo->jogador.posicao.y,LARGURA_JOGADOR, ALTURA_JOGADOR};
    for (int i = 0; i < jogo->jogador.salaAtual->qtdObstaculos; i++) {
        if (CheckCollisionRecs(bruno, jogo->jogador.salaAtual->obstaculos[i])) {
            return true;
        }
    }
    return false;
}

static int buscar_interativo_proximo(const Jogo *jogo) {
    // retângulo do Bruno "inflado" em DISTANCIA_INTERACAO para cada lado
    Rectangle alcance = {
        jogo->jogador.posicao.x - DISTANCIA_INTERACAO,
        jogo->jogador.posicao.y - DISTANCIA_INTERACAO,
        LARGURA_JOGADOR + 2 * DISTANCIA_INTERACAO,
        ALTURA_JOGADOR + 2 * DISTANCIA_INTERACAO
    };

    for (int i = 0; i < jogo->jogador.salaAtual->qntdInterativo; i++) {
        if (CheckCollisionRecs(alcance, jogo->jogador.salaAtual->interativos[i].area)) {
            return i;
        }
    }
    return -1;
}

/* ---------------- Salão: a charada do bilhete ----------------
   No verso do bilhete: "Tenho teclas, mas não abro portas. Tenho cauda,
   mas não sou bicho." -> o piano (de cauda). Objeto errado custa 2 min. */
static void interagir_objeto_errado_salao(Jogo *jogo, const char *descricao) {
    const char *falas[] = { descricao };
    if (jogo->leuBilhete && !jogo->salaoResolvido) {
        penalizar(jogo, 2, "não era isso");
    }
    abrir_dialogo(jogo, "Bruno", falas, 1);
}

static void interagir_piano(Jogo *jogo) {
    if (jogo->salaoResolvido) {
        static const char *falas[] = { "Já examinei o piano. Não há mais nada aqui." };
        abrir_dialogo(jogo, "Bruno", falas, QTD(falas));
        return;
    }
    if (!jogo->leuBilhete) {
        static const char *falas[] = { "Um piano de cauda. Bonito, mas não vejo nada de estranho... por enquanto." };
        abrir_dialogo(jogo, "Bruno", falas, QTD(falas));
        return;
    }
    static const char *falas[] = {
        "\"Tenho teclas, mas não abro portas. Tenho cauda, mas não sou bicho.\" É o piano!",
        "Escondidos dentro da tampa: a lista de convidados com os horários de chegada... e um bilhete do mordomo.",
        "\"22h05: dei corda no relógio do salão. Depois disso, a chave de corda sumiu.\"",
        "Alguém mexeu no relógio depois das 22h05. Os garçons do jardim devem ter visto algo."
    };
    abrir_dialogo(jogo, "Bruno", falas, QTD(falas));
    coletar_pista(jogo, "Lista de convidados: Otávio, Helena e o Sr. Almeida chegaram antes das 22h.", HORARIO(21, 30));
    coletar_pista(jogo, "O Sr. Almeida deu corda no relógio do salão; depois disso, a chave de corda sumiu.", HORARIO(22, 5));
    jogo->salaoResolvido = true;
}

//Jardim: garçons (puzzle) e a fonte (porão)
static void interagir_garcons(Jogo *jogo) {
    if (jogo->jardimResolvido) {
        static const char *falas[] = { "Os garçons já me disseram tudo o que sabiam." };
        abrir_dialogo(jogo, "Bruno", falas, QTD(falas));
        return;
    }
    // a ordem importa: sem a lista de convidados do salão, demora mais
    if (!jogo->salaoResolvido && !jogo->jardimPenalizado) {
        penalizar(jogo, 15, "sem a lista de convidados do salão");
        jogo->jardimPenalizado = true;
    }
    jardim_iniciar(&jogo->puzzleJardim);
    jogo->puzzleAtual = PUZZLE_JARDIM;
    jogo->estado = PUZZLE;
}

static void interagir_fonte(Jogo *jogo) {
    if (jogo->poraoAberto) {
        static const char *falas[] = { "A passagem para o porão continua aberta, à direita." };
        abrir_dialogo(jogo, "Bruno", falas, QTD(falas));
        return;
    }
    if (!jogo->galeriaResolvida) {
        static const char *falas[] = {
            "Uma fonte antiga. Quatro ladrilhos numerados em volta estão mais gastos: 1, 4, 7 e 8.",
            "Alguém pisa neles com frequência... mas em que ordem?"
        };
        abrir_dialogo(jogo, "Bruno", falas, QTD(falas));
        return;
    }
    static const char *falas[] = {
        "1847... o ano do quadro mais antigo! 1, 8, 4, 7.",
        "(Os ladrilhos afundam um a um. A fonte desliza e revela uma escada.)",
        "Uma passagem secreta para o porão. Quem plantou a bomba pode estar lá embaixo."
    };
    jogo_abrir_porao(jogo);
    abrir_dialogo(jogo, "Bruno", falas, QTD(falas));
}

// Galeria: os quadros do leilão (puzzle) 
static void interagir_quadros(Jogo *jogo) {
    if (jogo->galeriaResolvida) {
        static const char *falas[] = { "Os quadros já estão em ordem. A moldura do mais antigo continua aberta." };
        abrir_dialogo(jogo, "Bruno", falas, QTD(falas));
        return;
    }
    // a ordem importa: sem o depoimento do jardim, o jogador não ganha a dica
    if (!jogo->jardimResolvido && !jogo->galeriaPenalizada) {
        penalizar(jogo, 15, "sem o depoimento dos garçons");
        jogo->galeriaPenalizada = true;
    }
    galeria_iniciar(&jogo->puzzleGaleria, jogo->jardimResolvido);
    jogo->puzzleAtual = PUZZLE_GALERIA;
    jogo->estado = PUZZLE;
}

//Porão: confronto com Otávio (final muito bom) 
static void interagir_otavio(Jogo *jogo) {
    static const char *falas[] = {
        "Otávio: Bruno?! Como você achou este lugar?",
        "Bruno: Os ladrilhos, a moldura, as luvas. Você deixou um rastro, Otávio.",
        "(Antes que ele alcance o detonador, Bruno o imobiliza. O detonador cai e se apaga.)"
    };
    jogo->capturouNoPorao = true;
    jogo->terminarAposDialogo = true;
    jogo->resultadoPendente = final_decidir(true, false, false, false);   // muito bom
    abrir_dialogo(jogo, NULL, falas, QTD(falas));
}

static void jogo_interagir(Jogo *jogo) {
    const Interativo *obj = &jogo->jogador.salaAtual->interativos[jogo->interativoProximo];
    switch (obj->id) {
        case OBJ_BILHETE: {
            static const char *falas[] = {
                "(Lendo o bilhete) \"Uma bomba explodirá à meia-noite.\"",
                "TSC!",
                "Meia-noite... Tenho menos de uma hora. E não posso causar pânico entre os convidados.",
                "Quem escreveu isso sabe quem eu sou. Ou quer que eu jogue o jogo dele.",
                "Sem o traje, sem ajuda. Só eu e o que eu conseguir descobrir nesta mansão.",
                "(No verso) \"Tenho teclas, mas não abro portas. Tenho cauda, mas não sou bicho.\"",
                "Uma charada. A resposta deve estar aqui no salão."
            };
            abrir_dialogo(jogo, "Bruno", falas, QTD(falas));
            coletar_pista(jogo, "Bilhete: \"Uma bomba explodirá à meia-noite.\"", HORARIO(23, 0));
            jogo->leuBilhete = true;
            break;
        }
        case OBJ_RELOGIO: {
            if (!jogo->relogioLiberado) {
                static const char *falasTravado[] = {
                    "O relógio parou... e tem algo escondido no mecanismo.",
                    "Ainda não sei o suficiente. Preciso de mais pistas antes de mexer nisso."
                };
                abrir_dialogo(jogo, "Bruno", falasTravado, QTD(falasTravado));
            } else {
                // primeiro a acusação; quando ela termina, começa a bomba
                acusacao_iniciar(&jogo->acusacao, &jogo->jogador.inventario);
                jogo->estado = ACUSACAO;
            }
            break;
        }
        case OBJ_PIANO:      interagir_piano(jogo); break;
        case OBJ_TACA:       interagir_objeto_errado_salao(jogo, "Uma taça de champanhe pela metade. Nada de especial."); break;
        case OBJ_CARDAPIO:   interagir_objeto_errado_salao(jogo, "O cardápio da gala. Lagosta... não é hora para isso."); break;
        case OBJ_REPERTORIO: interagir_objeto_errado_salao(jogo, "O repertório da banda. Só valsas."); break;
        case OBJ_GARCONS:    interagir_garcons(jogo); break;
        case OBJ_FONTE:      interagir_fonte(jogo); break;
        case OBJ_QUADROS:    interagir_quadros(jogo); break;
        case OBJ_OTAVIO:     interagir_otavio(jogo); break;
        // acrescentem aqui o case do objeto de vcs
        default:
            break;
    }
}

static void atualizar_explorando(Jogo *jogo)
{
    float dt = GetFrameTime();
    float passo = VELOCIDADE_JOGADOR * dt;

    //movimentação
    float yAntigo = jogo->jogador.posicao.y;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
    {
        jogo->jogador.posicao.y -= passo;
    }
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
    {
        jogo->jogador.posicao.y += passo;
    }
    if(colide_com_sala(jogo)){
        jogo->jogador.posicao.y = yAntigo;
    }

    float xAntigo = jogo->jogador.posicao.x;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
    {
        jogo->jogador.posicao.x += passo;
    }
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
    {
        jogo->jogador.posicao.x -= passo;
    }
    if(colide_com_sala(jogo)){
        jogo->jogador.posicao.x = xAntigo;
    }

    //troca de sala
    int larguraTela = TELA_LARGURA;
    if (jogo->jogador.posicao.x + LARGURA_JOGADOR > larguraTela)
    {
        jogo->jogador.salaAtual = sala_ir_direita(jogo->jogador.salaAtual);
        jogo->jogador.posicao.x = 0;
    }
    if (jogo->jogador.posicao.x < 0)
    {
        jogo->jogador.salaAtual = sala_ir_esquerda(jogo->jogador.salaAtual);
        jogo->jogador.posicao.x = larguraTela - LARGURA_JOGADOR;
    }

    int alturaTela = TELA_ALTURA;
    //borda de cima
    if(jogo->jogador.posicao.y < 0){
        jogo->jogador.posicao.y = 0;
    }

    //borda de baixo
    if((jogo->jogador.posicao.y + ALTURA_JOGADOR) > alturaTela){
        jogo->jogador.posicao.y = alturaTela - ALTURA_JOGADOR;
    }

    //interação
    jogo->interativoProximo = buscar_interativo_proximo(jogo);
    if(jogo->interativoProximo != -1 && IsKeyPressed(KEY_E)){
        jogo_interagir(jogo);
    }
    if(IsKeyPressed(KEY_I)) {
        inventario_tela_abrir(&jogo->telaInventario, false);
        jogo->estado = INVENTARIO;
    }

    //teste para abrir o Porão
    // if(IsKeyPressed(KEY_P)){
    //     jogo_abrir_porao(jogo);
    // }

    //TESTE: libera o relógio sem resolver os puzzles (remover antes da entrega)
    // if(IsKeyPressed(KEY_L)){
    //     jogo_liberar_relogio(jogo);
    // }
}

static void atualizar_dialogo(Jogo *jogo) {
    if(dialogo_atualizar(&jogo->dialogo) ==  true){
        if (jogo->terminarAposDialogo) {           // ex.: confronto no porão
            jogo->terminarAposDialogo = false;
            jogo_terminar(jogo, jogo->resultadoPendente);
        } else {
            jogo->estado = EXPLORANDO;
        }
    }
}

// Puzzles do jardim e da galeria (estado PUZZLE) 
static void atualizar_puzzle(Jogo *jogo) {
    if (jogo->puzzleAtual == PUZZLE_JARDIM) {
        ResultadoPuzzle r = jardim_atualizar(&jogo->puzzleJardim);
        if (r == PZ_ACERTOU) {
            static const char *falas[] = {
                "Bruno: Edu, você acusou o Davi, mas o Fábio confirma tudo o que ele disse. Quem mente é você.",
                "Edu: Tá bom! Um convidado me pagou para dizer que ninguém foi à fonte...",
                "Davi: E tem mais: o Sr. Otávio passou a noite olhando para o quadro mais antigo da galeria."
            };
            coletar_pista(jogo, "Caio: a Sra. Helena saiu da galeria para atender um telefonema.", HORARIO(22, 10));
            coletar_pista(jogo, "Davi viu o Sr. Otávio mexendo na fonte do jardim.", HORARIO(22, 20));
            jogo->jardimResolvido = true;
            jogo->puzzleAtual = PUZZLE_NENHUM;
            abrir_dialogo(jogo, "Garçons", falas, QTD(falas));
        } else if (r == PZ_ERROU) {
            static const char *falas[] = {
                "Os garçons se entreolham. Acusei a pessoa errada... Preciso ouvir de novo com mais atenção."
            };
            penalizar(jogo, 5, "acusou o garçom errado");
            jogo->puzzleAtual = PUZZLE_NENHUM;
            abrir_dialogo(jogo, "Bruno", falas, QTD(falas));
        }
    } else if (jogo->puzzleAtual == PUZZLE_GALERIA) {
        ResultadoPuzzle r = galeria_atualizar(&jogo->puzzleGaleria);
        if (r == PZ_ACERTOU) {
            static const char *falas[] = {
                "Em ordem! E a moldura do quadro mais antigo, de 1847, está solta...",
                "Alguém a abriu com luvas: ficou uma marca de couro na madeira.",
                "E o Sr. Otávio... estava de luvas no começo da festa. Às 22h50 ele voltou ao salão sem elas.",
                "Já tenho o bastante. É hora de abrir o relógio do salão."
            };
            coletar_pista(jogo, "A moldura do quadro mais antigo foi aberta; há uma marca de luva nela.", HORARIO(22, 40));
            coletar_pista(jogo, "O Sr. Otávio voltou ao salão sem as luvas que usava no começo da festa.", HORARIO(22, 50));
            jogo->galeriaResolvida = true;
            jogo_liberar_relogio(jogo);
            jogo->puzzleAtual = PUZZLE_NENHUM;
            abrir_dialogo(jogo, "Bruno", falas, QTD(falas));
        } else if (r == PZ_SAIU) {
            jogo->puzzleAtual = PUZZLE_NENHUM;
            jogo->estado = EXPLORANDO;
        }
    } else {
        jogo->estado = EXPLORANDO;
    }
}

//tempo
static void atualizar_tempo(Jogo *jogo) {
    float dt = GetFrameTime();
    jogo->tempoRestante -= dt;
    if (jogo->tempoRestante <= 0)
    {
        jogo->tempoRestante = 0;
        jogo_terminar(jogo, RESULTADO_RUIM);   // deu meia-noite
    }
}

static void atualizar_inventario(Jogo *jogo) {
    if(inventario_tela_atualizar(&jogo->telaInventario, &jogo->jogador.inventario) == true){
        jogo->estado = EXPLORANDO;
    }
}

// Acusação: escolher o suspeito e marcar as provas. Ao confirmar, vai para a bomba.
static void atualizar_acusacao(Jogo *jogo) {
    if (acusacao_atualizar(&jogo->acusacao, &jogo->jogador.inventario)) {
        jogo->acusouCerto = jogo->acusacao.acertou;
        jogo->telaInventario.aberta = false;
        bomba_iniciar(&jogo->bomba);
        jogo->estado = BOMBA;
    }
}

// Bomba: cortar os fios. A tecla I abre a linha do tempo por cima, só para consulta.
static void atualizar_bomba(Jogo *jogo) {
    if (jogo->telaInventario.aberta) {
        inventario_tela_atualizar(&jogo->telaInventario, &jogo->jogador.inventario);
        return;
    }
    if (IsKeyPressed(KEY_I)) {
        inventario_tela_abrir(&jogo->telaInventario, false);
        return;
    }
    if (bomba_atualizar(&jogo->bomba)) {
        ResultadoFinal r = final_decidir(jogo->capturouNoPorao, jogo->acusouCerto,jogo->bomba.desarmada, false);
        jogo_terminar(jogo, r);
    }
}

// Tela de final: ENTER libera tudo e começa uma partida nova (volta ao menu)
static void atualizar_fim(Jogo *jogo) {
    if (IsKeyPressed(KEY_ENTER)) {
        jogo_finalizar(jogo);
        jogo_iniciar(jogo);
    }
}

void jogo_atualizar(Jogo *jogo)
{
    aviso_atualizar(GetFrameTime());
    if (jogo->estado == EXPLORANDO || jogo->estado == DIALOGO || jogo->estado == PUZZLE ||
        jogo->estado == INVENTARIO || jogo->estado == ACUSACAO || jogo->estado == BOMBA) {
        atualizar_tempo(jogo);
    }
    switch (jogo->estado)
    {
    case MENU:
        atualizar_menu(jogo);
        break;
    case EXPLORANDO:
        atualizar_explorando(jogo);
        break;
    case DIALOGO:
        atualizar_dialogo(jogo);
        break;
    case INVENTARIO:
        atualizar_inventario(jogo);
        break;
    case PUZZLE:
        atualizar_puzzle(jogo);
        break;
    case ACUSACAO:
        atualizar_acusacao(jogo);
        break;
    case BOMBA:
        atualizar_bomba(jogo);
        break;
    case FIM:
        atualizar_fim(jogo);
        break;
    default:
        break;
    }
}

// Painel escuro centralizado usado pelas telas de controles e créditos 
static Rectangle desenhar_painel_menu(const char *tituloPainel)
{
    Rectangle painel = { 290, 120, 700, 480 };
    DrawRectangle(0, 0, TELA_LARGURA, TELA_ALTURA, Fade(BLACK, 0.6f));
    DrawRectangleRec(painel, (Color){ 14, 13, 18, 240 });
    DrawRectangleLinesEx(painel, 3, (Color){ 200, 196, 188, 255 });
    int lt = MeasureText(tituloPainel, 34);
    DrawText(tituloPainel, (int)(painel.x + (painel.width - lt) / 2), (int)painel.y + 30, 34, (Color){ 230, 226, 218, 255 });
    const char *voltar = "ESC ou ENTER: voltar";
    DrawText(voltar, (int)(painel.x + (painel.width - MeasureText(voltar, 18)) / 2),(int)(painel.y + painel.height) - 40, 18, GRAY);
    return painel;
}

static void desenhar_menu_controles(void)
{
    Rectangle p = desenhar_painel_menu("CONTROLES");
    static const char *teclas[][2] = {
        { "WASD / Setas", "andar" },
        { "E",            "interagir, avançar diálogos" },
        { "I",            "linha do tempo (pistas)" },
        { "ESPAÇO",       "marcar pista na acusação" },
        { "ENTER",        "confirmar" },
        { "ESC",          "fechar telas" },
        { "F11",          "tela cheia" },
    };
    int y = (int)p.y + 110;
    for (int i = 0; i < QTD(teclas); i++, y += 42) {
        DrawText(teclas[i][0], (int)p.x + 90, y, 24, GOLD);
        DrawText(teclas[i][1], (int)p.x + 330, y, 24, RAYWHITE);
    }
}

static void desenhar_menu_creditos(void)
{
    Rectangle p = desenhar_painel_menu("CRÉDITOS");
    static const char *linhas[] = {
        "Matheus Costa da Rocha",
        "Luis Filipe Alves Silva Santos",
        "Amanda Campos Lima",
        "Leticia Camello Barros",
        "",
        "Algoritmos e Estruturas de Dados - CESAR School",
        "Feito em C com raylib",
    };
    int y = (int)p.y + 110;
    for (int i = 0; i < QTD(linhas); i++, y += 40) {
        int tam = (i < 4) ? 26 : 20;
        Color cor = (i < 4) ? RAYWHITE : LIGHTGRAY;
        DrawText(linhas[i], (int)(p.x + (p.width - MeasureText(linhas[i], tam)) / 2), y, tam, cor);
    }
}

static void desenhar_menu(const Jogo *jogo)
{
    if (fundoMenuOk) {
        // imagem de fundo esticada para a resolução fixa
        DrawTexturePro(fundoMenu,
                       (Rectangle){ 0, 0, (float)fundoMenu.width, (float)fundoMenu.height },
                       (Rectangle){ 0, 0, TELA_LARGURA, TELA_ALTURA },
                       (Vector2){ 0, 0 }, 0.0f, WHITE);

        if (jogo->telaMenu == TELA_MENU_CONTROLES) { desenhar_menu_controles(); return; }
        if (jogo->telaMenu == TELA_MENU_CREDITOS)  { desenhar_menu_creditos();  return; }

        // brilho no botão selecionado (os botões já estão desenhados na imagem)
        Rectangle b = BOTOES_MENU[jogo->opcaoMenu];
        DrawRectangleRec(b, Fade(WHITE, 0.10f));
        DrawRectangleRoundedLinesEx((Rectangle){ b.x - 4, b.y - 4, b.width + 8, b.height + 8 },
                                    0.15f, 6, 3.0f, (Color){ 245, 240, 230, 255 });
        DrawText(controles, (TELA_LARGURA - MeasureText(controles, 16)) / 2, TELA_ALTURA - 30, 16, Fade(LIGHTGRAY, 0.8f));
        return;
    }

    /* ---- sem a imagem (arquivo não encontrado): menu só com texto ---- */
    if (jogo->telaMenu == TELA_MENU_CONTROLES) { desenhar_menu_controles(); return; }
    if (jogo->telaMenu == TELA_MENU_CREDITOS)  { desenhar_menu_creditos();  return; }
    int larguraTela = TELA_LARGURA;
    int alturaTela = TELA_ALTURA;
    int larguraTexto = MeasureText(titulo, tamanhoFonte);

    int x = (larguraTela - larguraTexto) / 2;
    int y = (alturaTela - tamanhoFonte) / 2;

    DrawText(titulo, x, y, tamanhoFonte, WHITE);

    int larguraSub = MeasureText(subtitulo, tamanhoFonteSub);

    int xSub = (larguraTela - larguraSub) / 2;
    int ySub = y + 60;

    DrawText(subtitulo, xSub, ySub, tamanhoFonteSub, GRAY);

    // botões em texto, com o selecionado destacado
    for (int i = 0; i < QTD_OPCOES_MENU; i++) {
        const char *r = ROTULOS_MENU[i];
        Color cor = (i == jogo->opcaoMenu) ? GOLD : GRAY;
        DrawText(r, (larguraTela - MeasureText(r, 24)) / 2, ySub + 50 + i * 40, 24, cor);
    }

    int larguraControles = MeasureText(controles, 18);
    DrawText(controles, (larguraTela - larguraControles) / 2, alturaTela - 60, 18, DARKGRAY);
}

// Relógio da bomba (23:00 -> 00:00), alinhado pela direita em xDireita 
static void desenhar_relogio(const Jogo *jogo, int xDireita, int y)
{
    float tempoPassado = TEMPO_TOTAL - jogo->tempoRestante;
    int minutos = tempoPassado * 60 / TEMPO_TOTAL;

    const char *relogio;
    if (minutos < 60)
    {
        relogio = TextFormat("23:%02d", minutos);
    }
    else
    {
        relogio = "00:00";
    }

    int tamanhoRelogio = 30;
    int larguraRelogio = MeasureText(relogio, tamanhoRelogio);
    Color corRelogio = WHITE;
    if (minutos >= 50)
    {
        corRelogio = RED;
    }
    DrawText(relogio, xDireita - larguraRelogio, y, tamanhoRelogio, corRelogio);
}

static void desenhar_explorando(const Jogo *jogo)
{
    int larguraTela = TELA_LARGURA;
    int alturaTela = TELA_ALTURA;

    // fundo com a cor da sala atual
    DrawRectangle(0, 0, larguraTela, alturaTela, jogo->jogador.salaAtual->corFundo);

    const Sala *sala = jogo->jogador.salaAtual;

    //obstaculo desenho: os NUM_PAREDES primeiros são as paredes; o resto, móveis
    Color corParede = ColorBrightness(sala->corFundo, -0.55f);
    for (int i = 0; i < sala->qtdObstaculos; i++) {
        DrawRectangleRec(sala->obstaculos[i], i < NUM_PAREDES ? corParede : DARKGRAY);
    }

    // portas: as aberturas nas paredes laterais
    DrawRectangle(0, PORTA_Y, PAREDE_ESPESSURA, PORTA_ALTURA, Fade(GOLD, 0.35f));
    DrawRectangle(larguraTela - PAREDE_ESPESSURA, PORTA_Y, PAREDE_ESPESSURA, PORTA_ALTURA, Fade(GOLD, 0.35f));

    //interativo desenho
    for(int i = 0; i < sala->qntdInterativo; i++){
        DrawRectangleRec(sala->interativos[i].area, Fade(YELLOW, 0.85f));
    }

    // nome da sala no canto superior esquerdo (por cima da parede)
    DrawText(sala->nome, 40, 34, 30, WHITE);

    
    
    // jogador (BRUNO)
    DrawRectangle((int)jogo->jogador.posicao.x, (int)jogo->jogador.posicao.y, LARGURA_JOGADOR, ALTURA_JOGADOR, BLUE);
    
    //interativo aviso
    if (jogo->interativoProximo != -1 && jogo->estado == EXPLORANDO) {
        const Interativo *obj =  &jogo->jogador.salaAtual->interativos[jogo->interativoProximo];
        const char *aviso = TextFormat("[E] %s", obj->rotulo);
        DrawText(aviso, (int)jogo->jogador.posicao.x, ((int)jogo->jogador.posicao.y - 25),  20, GOLD);
    }
    
    desenhar_relogio(jogo, larguraTela - 40, 34);
}

static void desenhar_dialogo(const Jogo *jogo) {
    desenhar_explorando(jogo);   // a sala continua aparecendo por trás
    dialogo_desenhar(&jogo->dialogo);
}


static void desenhar_inventario(const Jogo *jogo) {
    desenhar_explorando(jogo);
    inventario_tela_desenhar(&jogo->telaInventario, &jogo->jogador.inventario);
}

static void desenhar_puzzle(const Jogo *jogo) {
    desenhar_explorando(jogo);
    if (jogo->puzzleAtual == PUZZLE_JARDIM)        jardim_desenhar(&jogo->puzzleJardim);
    else if (jogo->puzzleAtual == PUZZLE_GALERIA)  galeria_desenhar(&jogo->puzzleGaleria);
}

static void desenhar_acusacao(const Jogo *jogo) {
    desenhar_explorando(jogo);
    DrawRectangle(0, 0, TELA_LARGURA, TELA_ALTURA, Fade(BLACK, 0.6f));
    acusacao_desenhar(&jogo->acusacao, &jogo->jogador.inventario);
}

static void desenhar_bomba(const Jogo *jogo) {
    desenhar_explorando(jogo);   // mantém o relógio da bomba visível no canto
    DrawRectangle(0, 0, TELA_LARGURA, TELA_ALTURA, Fade(BLACK, 0.85f));
    bomba_desenhar(&jogo->bomba);
    desenhar_relogio(jogo, TELA_LARGURA - 40, TELA_ALTURA - 70);   // o tempo continua correndo
    inventario_tela_desenhar(&jogo->telaInventario, &jogo->jogador.inventario); // só aparece se aberta
}

static void desenhar_fim(const Jogo *jogo) {
    final_desenhar(jogo->resultado);
}

void jogo_desenhar(const Jogo *jogo)
{
    switch (jogo->estado)
    {
    case MENU:
        desenhar_menu(jogo);
        break;
    case EXPLORANDO:
        desenhar_explorando(jogo);
        break;
    case DIALOGO:
        desenhar_dialogo(jogo);
        break;
    case INVENTARIO:
        desenhar_inventario(jogo);
        break;
    case PUZZLE:
        desenhar_puzzle(jogo);
        break;
    case ACUSACAO:
        desenhar_acusacao(jogo);
        break;
    case BOMBA:
        desenhar_bomba(jogo);
        break;
    case FIM:
        desenhar_fim(jogo);
        break;
    default:
        break;
    }
    aviso_desenhar();
}

void jogo_finalizar(Jogo *jogo)
{
    jogador_finalizar(&jogo->jogador);
    salas_liberar(jogo->salas);
    jogo->salas = NULL;
    jogo->jogador.salaAtual = NULL;
}

void jogo_penalizar(Jogo *jogo, int minutosJogo) {
    float segundos = minutosJogo * TEMPO_TOTAL / 60.0f;
    jogo->tempoRestante -= segundos;
}

void jogo_abrir_porao(Jogo *jogo) {
    if(jogo->poraoAberto == true){
        return;
    }
    Sala *jardim = jogo->salas->proxSala;
    Sala *porao  = sala_inserir_depois(jardim, "Porão", (Color){ 12, 15, 25, 255 });
    if(porao != NULL){
        // caixas de contrabando e o culpado com o detonador
        sala_adicionar_obstaculo(porao, (Rectangle){ 300, 150, 90, 90 });
        sala_adicionar_obstaculo(porao, (Rectangle){ 700, 500, 90, 90 });
        Rectangle otavio = { 900, 336, 32, 48 };
        sala_adicionar_obstaculo(porao, otavio);
        sala_adicionar_interativo(porao, OBJ_OTAVIO, otavio, "Confrontar Otávio");
        jogo->poraoAberto = true;
    }
}

// Libera o relógio do salão (acusação + bomba). Chamar quando o último puzzle for resolvido.
void jogo_liberar_relogio(Jogo *jogo) {
    jogo->relogioLiberado = true;
}
