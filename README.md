# IBatYouCantSolveThis

## Pessoa 1:

## Pessoa 2: inventário, telas genéricas e final

| Arquivo | O que tem |
|---|---|
| `src/jogador.c` | implementação do inventário (lista encadeada simples, inserção ordenada por horário) |
| `include/ui.h`, `src/ui.c` | caixa de diálogo (fila de falas), escolher entre N opções, aviso na tela |
| `include/inventario_tela.h`, `src/inventario_tela.c` | tela da linha do tempo (tecla I) com marcação |
| `include/final.h`, `src/final.c` | acusação, corte dos fios e telas dos finais |

Uso rápido:

    jogador_coletar_fala(&jogador, "22h40: Otavio visto saindo da galeria", HORARIO(22,40));
    aviso_mostrar("Pista adicionada", 1.5f);
    if (!inventario_tem_horario(&jogador.inventario, HORARIO(21,30))) jogo_penalizar(jogo, 15);

    dialogo_abrir(&dlg, "Garcom", falas, 3);    // update: dialogo_atualizar  draw: dialogo_desenhar
    escolha_abrir(&esc, "Quem mente?", nomes, 4); // int r = escolha_atualizar(&esc); r >= 0 ao escolher

    ResultadoFinal r = final_decidir(capturouNoPorao, acusacao.acertou, bomba.desarmada, tempoAcabou);

Culpado, provas e ordem dos fios ficam em `include/final.h` / `src/final.c`
(`CULPADO`, `PROVAS`, `ORDEM_FIOS`), identificados pelo horário de cada pista.

## Pessoa 3:

## Pessoa 4: