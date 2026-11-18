#include <stdlib.h>
#include "jogo.h"
#include "jogador_anim.h"

#define DISTANCIA_INTERACAO 20

#define QTD(v) ((int)(sizeof(v) / sizeof((v)[0])))   // quantidade de itens de um vetor

static const char *titulo = "I Bat You Can't Solve It";
static const char *subtitulo = "Pressione ENTER para começar";
static const char *controles = "WASD/Setas: andar   E: interagir   I: linha do tempo   F11: tela cheia";
static const int tamanhoFonte = 40;
static const int tamanhoFonteSub = 20;

//recursos (imagens)
static Texture2D fundoMenu;
static bool fundoMenuOk = false;   // se a imagem não carregar, o menu usa só texto
static Texture2D spriteMordomo;    // 64x32: 2 quadros de 32x32 (parado, respirando)
static Texture2D spriteOtavio;     // empresario.png
static Texture2D spriteGarcom;     // garcom.png (mesmo formato do mordomo)
static Texture2D spriteGarcom2;    // garcom2.png (outro garçom, cores diferentes)
static Texture2D spriteConvidado[3]; // convidado1..3.png: convidados sentados nas mesas
static bool mostrarColisao = false; // F3: mostra as caixas de colisão (para ajustar o mapa)
static Music musica;               // trilha de fundo em loop (assets/musica.ogg)
static bool musicaOk = false;      // false se não houver som: o jogo segue mudo
static bool musicaMuda = false;    // tecla M
#define VOLUME_MUSICA 0.01f

static Texture2D carregar_pixel_art(const char *caminho) {
    Texture2D t = LoadTexture(caminho);
    if (t.id != 0) SetTextureFilter(t, TEXTURE_FILTER_POINT);
    return t;
}

void jogo_carregar_recursos(void) {
    fundoMenu = LoadTexture("assets/menu.png");
    fundoMenuOk = (fundoMenu.id != 0);
    if (fundoMenuOk) SetTextureFilter(fundoMenu, TEXTURE_FILTER_BILINEAR);
    spriteMordomo = carregar_pixel_art("assets/mordomo.png");
    spriteOtavio  = carregar_pixel_art("assets/empresario.png");
    spriteGarcom  = carregar_pixel_art("assets/garcom.png");
    spriteGarcom2 = carregar_pixel_art("assets/garcom2.png");
    for (int i = 0; i < 3; i++)
        spriteConvidado[i] = carregar_pixel_art(TextFormat("assets/convidado%d.png", i + 1));
    jogador_anim_iniciar((Vector2){ 0, 0 });

    InitAudioDevice();
    if (IsAudioDeviceReady()) {
        musica = LoadMusicStream("assets/musica.ogg");
        musicaOk = (musica.frameCount > 0);
        if (musicaOk) {
            musica.looping = true;
            SetMusicVolume(musica, VOLUME_MUSICA);
            PlayMusicStream(musica);
        }
    }

}

void jogo_liberar_recursos(void) {
    if (fundoMenuOk) UnloadTexture(fundoMenu);
    fundoMenuOk = false;
    if (spriteMordomo.id != 0) UnloadTexture(spriteMordomo);
    if (spriteOtavio.id != 0)  UnloadTexture(spriteOtavio);
    if (spriteGarcom.id != 0)  UnloadTexture(spriteGarcom);
    if (spriteGarcom2.id != 0) UnloadTexture(spriteGarcom2);
    for (int i = 0; i < 3; i++)
        if (spriteConvidado[i].id != 0) UnloadTexture(spriteConvidado[i]);
    jogador_anim_finalizar();
    if (musicaOk) UnloadMusicStream(musica);
    musicaOk = false;
    if (IsAudioDeviceReady()) CloseAudioDevice();
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
    jogo->estadoAnterior = MENU;
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

/*Salão: a charada do bilhete 
   No verso do bilhete: "Tenho cúpula, mas não sou igreja. Guardo o jantar,
   mas não sou geladeira." -> a campânula de prata do bufê.
   Objeto errado custa 2 min. */
static void interagir_objeto_errado_salao(Jogo *jogo, const char *descricao) {
    const char *falas[] = { descricao };
    if (jogo->leuBilhete && !jogo->salaoResolvido) {
        penalizar(jogo, 2, "não era isso");
    }
    abrir_dialogo(jogo, "Bruno", falas, 1);
}

static void interagir_campanula(Jogo *jogo) {
    if (jogo->salaoResolvido) {
        static const char *falas[] = { "Já olhei embaixo da campânula. Não há mais nada aqui." };
        abrir_dialogo(jogo, "Bruno", falas, QTD(falas));
        return;
    }
    if (!jogo->leuBilhete) {
        static const char *falas[] = { "Uma campânula de prata sobre o bufê. Bonita, mas não vejo nada de estranho... por enquanto." };
        abrir_dialogo(jogo, "Bruno", falas, QTD(falas));
        return;
    }
    static const char *falas[] = {
        "\"Tenho cúpula, mas não sou igreja. Guardo o jantar, mas não sou geladeira.\" É a campânula!",
        "Embaixo dela, no lugar do assado: a lista de convidados com os horários de chegada... e um bilhete do mordomo.",
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
                "(O mordomo entrega um envelope) \"Deixaram isto para o senhor, Sr. Bruno.\"",
                "(Lendo o bilhete) \"Uma bomba explodirá à meia-noite.\"",
                "TSC!",
                "Meia-noite... Tenho menos de uma hora. E não posso causar pânico entre os convidados.",
                "Quem escreveu isso sabe quem eu sou. Ou quer que eu jogue o jogo dele.",
                "Sem o traje, sem ajuda. Só eu e o que eu conseguir descobrir nesta mansão.",
                "(No verso) \"Tenho cúpula, mas não sou igreja. Guardo o jantar, mas não sou geladeira.\"",
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
        case OBJ_CAMPANULA:  interagir_campanula(jogo); break;
        case OBJ_PRATOS:     interagir_objeto_errado_salao(jogo, "Pratos de porcelana empilhados. Nada embaixo deles."); break;
        case OBJ_FLORES:     interagir_objeto_errado_salao(jogo, "Um arranjo de flores na mesa. Só flores."); break;
        case OBJ_VASO:       interagir_objeto_errado_salao(jogo, "Um vaso de planta. Só terra."); break;
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

    //troca de sala: encostou no gatilho de uma porta -> vai para a sala ligada a ela
    Sala *atual = jogo->jogador.salaAtual;
    Rectangle bruno = { jogo->jogador.posicao.x, jogo->jogador.posicao.y, LARGURA_JOGADOR, ALTURA_JOGADOR };
    if (CheckCollisionRecs(bruno, atual->portaProx.gatilho)) {
        Sala *destino = sala_ir_direita(atual);
        jogo->jogador.salaAtual = destino;
        jogo->jogador.posicao = destino->portaAnt.chegada;    // entra pela porta que vem de 'atual'
    } else if (CheckCollisionRecs(bruno, atual->portaAnt.gatilho)) {
        Sala *destino = sala_ir_esquerda(atual);
        jogo->jogador.salaAtual = destino;
        jogo->jogador.posicao = destino->portaProx.chegada;
    }

    //bordas da tela (segurança: as paredes já seguram o Bruno)
    int larguraTela = TELA_LARGURA;
    if (jogo->jogador.posicao.x < 0) jogo->jogador.posicao.x = 0;
    if (jogo->jogador.posicao.x + LARGURA_JOGADOR > larguraTela) jogo->jogador.posicao.x = larguraTela - LARGURA_JOGADOR;

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
    if(IsKeyPressed(KEY_F3)) mostrarColisao = !mostrarColisao;
    jogador_anim_atualizar(jogo->jogador.posicao, dt);
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

/* ---------------- pausa (tecla P) ----------------
   Só pausa durante a partida (não no menu nem na tela de final).
   O estado anterior é guardado para voltar exatamente de onde parou. */
static bool pode_pausar(EstadoJogo estado) {
    return estado == EXPLORANDO || estado == DIALOGO || estado == PUZZLE ||
           estado == INVENTARIO || estado == ACUSACAO || estado == BOMBA;
}

static void alternar_pausa(Jogo *jogo) {
    if (jogo->estado == PAUSADO) {
        jogo->estado = jogo->estadoAnterior;
    } else if (pode_pausar(jogo->estado)) {
        jogo->estadoAnterior = jogo->estado;
        jogo->estado = PAUSADO;
    }
}

void jogo_atualizar(Jogo *jogo)
{
    // música: precisa ser alimentada todo frame; M liga/desliga
    if (musicaOk) {
        UpdateMusicStream(musica);
        if (IsKeyPressed(KEY_M)) {
            musicaMuda = !musicaMuda;
            SetMusicVolume(musica, musicaMuda ? 0.0f : VOLUME_MUSICA);
        }
    }

    // P vale em qualquer estado e vem antes do relógio, para a pausa já congelar este frame
    if (IsKeyPressed(KEY_P)) {
        alternar_pausa(jogo);
    }
    if (jogo->estado == PAUSADO) return;   // nada anda enquanto pausado

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
        { "ENTER",        "confirmar, marcar pista" },
        { "ESPAÇO",       "enviar a acusação" },
        { "ESC",          "fechar telas" },
        { "P",            "pausar" },
        { "M",            "música liga/desliga" },
        { "F11",          "tela cheia" },
    };
    int y = (int)p.y + 110;
    for (int i = 0; i < QTD(teclas); i++, y += 38) {
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

/* NPC parado: folha 64x32 com 2 quadros de 32x32 alternando devagar.
   'pes' é a caixa de colisão; o sprite (3x) fica centralizado com os pés nela. */
static void desenhar_npc(Texture2D folha, Rectangle pes) {
    if (folha.id == 0) { DrawRectangleRec(pes, MAROON); return; }
    int quadro = ((int)(GetTime() * 2.0)) % 2;
    float lado = 32.0f * ESCALA_MUNDO;
    Rectangle origem  = { quadro * 32.0f, 0, 32, 32 };
    Rectangle destino = { pes.x + pes.width / 2 - lado / 2, pes.y + pes.height - lado, lado, lado };
    DrawTexturePro(folha, origem, destino, (Vector2){ 0, 0 }, 0, WHITE);
}

// Personagens que aparecem em cada sala (os pés coincidem com obstáculos de sala.c / jogo_abrir_porao)
#define PES_MORDOMO ((Rectangle){ 704, 262, 32, 20 })
#define PES_OTAVIO  ((Rectangle){ 900, 360, 32, 24 })
#define PES_GARCOM1 ((Rectangle){ 205, 428, 32, 24 })
#define PES_GARCOM2 ((Rectangle){ 262, 434, 32, 24 })

/* ---------- convidados sentados (só decoração, sem colisão nem diálogo) ----------
   'assento' é o ponto do assento da cadeira: centro em x, altura do quadril em y.
   sala: 0 = salão, 1 = jardim. */
typedef struct {
    int sala;
    int sprite;          // índice em spriteConvidado
    Vector2 assento;
    bool espelhar;       // vira o sprite para olhar para a mesa
} Convidado;

static const Convidado convidados[] = {
    { 0, 0, { 152, 222 }, false },   // salão, mesa de cima à esquerda
    { 0, 1, { 1163, 226 }, true },   // salão, mesa de cima à direita
    { 0, 2, { 180, 560 }, false },   // salão, mesa de baixo à esquerda
    { 0, 0, { 1110, 568 }, true },   // salão, mesa de baixo à direita
    { 1, 1, { 355, 250 }, false },   // jardim, banco de cima à esquerda
    { 1, 2, { 935, 250 }, true },    // jardim, banco de cima à direita
    { 1, 0, { 880, 660 }, false },   // jardim, banco de baixo à direita
};
#define LINHA_QUADRIL 26   // linha da folha 32x32 onde o corpo termina (corta as pernas)

static void desenhar_convidado(const Convidado *c) {
    Texture2D folha = spriteConvidado[c->sprite];
    if (folha.id == 0) return;
    int quadro = ((int)(GetTime() * 1.5 + c->sprite)) % 2;
    float lado = 32.0f * ESCALA_MUNDO;
    Rectangle origem  = { quadro * 32.0f, 0, c->espelhar ? -32.0f : 32.0f, LINHA_QUADRIL };
    Rectangle destino = { c->assento.x - lado / 2, c->assento.y - LINHA_QUADRIL * ESCALA_MUNDO,
                          lado, LINHA_QUADRIL * ESCALA_MUNDO };
    DrawTexturePro(folha, origem, destino, (Vector2){ 0, 0 }, 0, WHITE);
}

static void desenhar_personagens(const Jogo *jogo) {
    const Sala *sala = jogo->jogador.salaAtual;
    Texture2D folhas[2];
    Rectangle pes[2];
    int qtd = 0;
    if (sala == jogo->salas) {                                   // salão
        folhas[qtd] = spriteMordomo; pes[qtd++] = PES_MORDOMO;
    } else if (sala == jogo->salas->proxSala) {                  // jardim: garçons atrás do balcão
        folhas[qtd] = spriteGarcom; pes[qtd++] = PES_GARCOM1;
        folhas[qtd] = spriteGarcom2; pes[qtd++] = PES_GARCOM2;
    } else if (jogo->poraoAberto && sala == jogo->salas->proxSala->proxSala) {   // porão (logo depois do jardim)
        folhas[qtd] = spriteOtavio; pes[qtd++] = PES_OTAVIO;
    }

    int salaConvidados = (sala == jogo->salas) ? 0 : (sala == jogo->salas->proxSala) ? 1 : -1;

    // quem está mais "para baixo" na tela é desenhado por cima (profundidade)
    float pesBruno = jogo->jogador.posicao.y + ALTURA_JOGADOR;
    for (int i = 0; i < QTD(convidados); i++)
        if (convidados[i].sala == salaConvidados && convidados[i].assento.y <= pesBruno) desenhar_convidado(&convidados[i]);
    for (int i = 0; i < qtd; i++)
        if (pes[i].y + pes[i].height <= pesBruno) desenhar_npc(folhas[i], pes[i]);
    jogador_anim_desenhar(jogo->jogador.posicao, LARGURA_JOGADOR, ALTURA_JOGADOR);
    for (int i = 0; i < qtd; i++)
        if (pes[i].y + pes[i].height > pesBruno) desenhar_npc(folhas[i], pes[i]);
    for (int i = 0; i < QTD(convidados); i++)
        if (convidados[i].sala == salaConvidados && convidados[i].assento.y > pesBruno) desenhar_convidado(&convidados[i]);
}

static void desenhar_explorando(const Jogo *jogo)
{
    int larguraTela = TELA_LARGURA;
    const Sala *sala = jogo->jogador.salaAtual;

    if (sala->fundo.id != 0) {
        // imagem 640x360 esticada para 1280x720 (2x, pixel art)
        DrawTexturePro(sala->fundo,
                       (Rectangle){ 0, 0, (float)sala->fundo.width, (float)sala->fundo.height },
                       (Rectangle){ 0, 0, TELA_LARGURA, TELA_ALTURA }, (Vector2){ 0, 0 }, 0, WHITE);
    } else {
        // sala sem imagem: cor de fundo, paredes e móveis em blocos
        DrawRectangle(0, 0, TELA_LARGURA, TELA_ALTURA, sala->corFundo);
        for (int i = 0; i < sala->qtdObstaculos; i++) {
            DrawRectangleRec(sala->obstaculos[i], i < 6 ? ColorBrightness(sala->corFundo, -0.55f) : DARKGRAY);
        }
        DrawRectangleRec(sala->portaAnt.gatilho,  Fade(GOLD, 0.35f));
        DrawRectangleRec(sala->portaProx.gatilho, Fade(GOLD, 0.35f));
    }

    // passagens que não aparecem no desenho (ex.: laterais da galeria): faixa escura + seta
    const Porta *portas[2] = { &sala->portaAnt, &sala->portaProx };
    for (int i = 0; i < 2; i++) {
        if (!portas[i]->marcar || sala->fundo.id == 0) continue;
        Rectangle g = portas[i]->gatilho;
        bool vertical = g.height > g.width;   // porta na lateral
        Rectangle faixa = vertical ? (Rectangle){ g.x < TELA_LARGURA / 2 ? 0 : TELA_LARGURA - 48, g.y, 48, g.height }
                                   : (Rectangle){ g.x, TELA_ALTURA - 48, g.width, 48 };
        DrawRectangleRec(faixa, Fade(BLACK, 0.6f));
        DrawRectangleLinesEx(faixa, 2, Fade(GOLD, 0.7f));
        const char *seta = vertical ? (g.x < TELA_LARGURA / 2 ? "<" : ">") : "v";
        int ls = MeasureText(seta, 44);
        DrawText(seta, (int)(faixa.x + faixa.width / 2 - ls / 2), (int)(faixa.y + faixa.height / 2 - 22), 44, GOLD);
    }

    if (mostrarColisao) {   // F3
        for (int i = 0; i < sala->qtdObstaculos; i++) DrawRectangleLinesEx(sala->obstaculos[i], 2, RED);
        for (int i = 0; i < sala->qntdInterativo; i++) DrawRectangleLinesEx(sala->interativos[i].area, 2, YELLOW);
        DrawRectangleRec(sala->portaAnt.gatilho,  Fade(SKYBLUE, 0.6f));
        DrawRectangleRec(sala->portaProx.gatilho, Fade(SKYBLUE, 0.6f));
        DrawRectangleLines((int)jogo->jogador.posicao.x, (int)jogo->jogador.posicao.y, LARGURA_JOGADOR, ALTURA_JOGADOR, GREEN);
    }

    // destaca o objeto perto do Bruno
    if (jogo->interativoProximo != -1) {
        DrawRectangleLinesEx(sala->interativos[jogo->interativoProximo].area, 2, Fade(GOLD, 0.8f));
    }

    desenhar_personagens(jogo);

    // nome da sala no canto superior esquerdo
    DrawText(sala->nome, 40, 34, 30, WHITE);

    //interativo aviso (acima da cabeça do Bruno, sem sair da tela)
    if (jogo->interativoProximo != -1 && jogo->estado == EXPLORANDO) {
        const Interativo *obj = &sala->interativos[jogo->interativoProximo];
        const char *aviso = TextFormat("[E] %s", obj->rotulo);
        int largura = MeasureText(aviso, 20);
        int x = (int)(jogo->jogador.posicao.x + LARGURA_JOGADOR / 2) - largura / 2;
        int y = (int)jogo->jogador.posicao.y - 32 * ESCALA_MUNDO + ALTURA_JOGADOR - 28;
        if (x < 8) x = 8;
        if (x + largura > larguraTela - 8) x = larguraTela - 8 - largura;
        if (y < 80) y = 80;
        DrawRectangle(x - 8, y - 4, largura + 16, 28, Fade(BLACK, 0.65f));
        DrawText(aviso, x, y, 20, GOLD);
    }

    //desenha relogio
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

static void desenhar_estado(const Jogo *jogo, EstadoJogo estado)
{
    switch (estado)
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
}

static void desenhar_pausado(const Jogo *jogo)
{
    desenhar_estado(jogo, jogo->estadoAnterior);   // a cena congelada continua por trás
    DrawRectangle(0, 0, TELA_LARGURA, TELA_ALTURA, Fade(BLACK, 0.65f));

    const char *texto = "PAUSADO";
    DrawText(texto, (TELA_LARGURA - MeasureText(texto, 60)) / 2, TELA_ALTURA / 2 - 60, 60, WHITE);
    const char *dica = "[P] continuar";
    DrawText(dica, (TELA_LARGURA - MeasureText(dica, 22)) / 2, TELA_ALTURA / 2 + 20, 22, LIGHTGRAY);
}

void jogo_desenhar(const Jogo *jogo)
{
    if (jogo->estado == PAUSADO) {
        desenhar_pausado(jogo);
    } else {
        desenhar_estado(jogo, jogo->estado);
        aviso_desenhar();
    }
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
        sala_montar_porao(porao);   // imagem, paredes e portas (sala.c)
        // o culpado com o detonador
        Rectangle otavio = { 900, 360, 32, 24 };   // pés do Otávio (sprite desenhado em jogo.c)
        sala_adicionar_obstaculo(porao, otavio);
        sala_adicionar_interativo(porao, OBJ_OTAVIO, otavio, "Confrontar Otávio");
        jogo->poraoAberto = true;
    }
}

// Libera o relógio do salão (acusação + bomba). Chamar quando o último puzzle for resolvido.
void jogo_liberar_relogio(Jogo *jogo) {
    jogo->relogioLiberado = true;
}
