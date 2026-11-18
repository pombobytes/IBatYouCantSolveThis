# I Bat You Can't Solve It

Jogo de mistério em C com [raylib](https://www.raylib.com/), feito para a disciplina de **Algoritmos e Estruturas de Dados** da CESAR School.

Bruno, um bilionário que à noite é vigilante, está numa gala na mansão quando o mordomo lhe entrega um bilhete: **uma bomba vai explodir à meia-noite**. Sem traje e sem ajuda, ele tem até 00:00 (10 minutos reais) para seguir as pistas pelas salas da mansão, descobrir quem plantou a bomba e desarmá-la.

---

## Como compilar e rodar

### Linux / WSL (Ubuntu)

1. Instale as dependências e a raylib (só na primeira vez):

   ```bash
   sudo apt update
   sudo apt install -y build-essential git libasound2-dev libx11-dev libxrandr-dev libxi-dev \
                       libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev
   git clone --depth 1 https://github.com/raysan5/raylib.git
   cd raylib/src && make PLATFORM=PLATFORM_DESKTOP && sudo make install && cd ../..
   ```

2. Na pasta do projeto:

   ```bash
   make run
   ```

> **Sem som no WSL?** Instale `sudo apt install -y libpulse0 libasound2-plugins`. Se o computador não tiver saída de som, o jogo funciona normalmente, só que sem música.

### Windows (MinGW)

Com a raylib em `C:/raylib/raylib/src` (caminho usado no `Makefile`):

```bash
mingw32-make run
```

Para apagar o executável: `make clean` (ou `mingw32-make clean`).

---

## Controles

| Tecla | Ação |
|---|---|
| **WASD / Setas** | andar |
| **E** | interagir, avançar diálogos |
| **I** | linha do tempo (pistas coletadas) |
| **ENTER** (ou clique) | confirmar; na acusação, marcar/desmarcar uma pista |
| **ESPAÇO** | enviar a acusação |
| **ESC** | fechar telas |
| **P** | pausar (o relógio para) |
| **M** | música liga/desliga |
| **F11** | tela cheia |
| **F10** | sair|

O menu também funciona com o mouse.

---

## Como se joga

O relógio começa em **23:00**. Cada sala tem um puzzle e cada puzzle resolvido rende pistas, guardadas na **linha do tempo** em ordem de horário.

1. **Salão de Festas:** o mordomo entrega o bilhete, que traz uma charada no verso. A resposta está em algum objeto do salão. Cada objeto errado custa 2 minutos.
2. **Jardim de Inverno:** quatro garçons dão seus depoimentos, e um deles está mentindo. Acusar o garçom errado custa 5 minutos.
3. **Galeria de Arte:** os 6 quadros do leilão precisam ser colocados em ordem de ano, como no insertion sort: pegar um quadro e inseri-lo na posição certa. Resolver a galeria libera o relógio do salão.
4. **Relógio do salão:** é onde a bomba está escondida. Primeiro Bruno escolhe o suspeito e marca as provas na linha do tempo; depois corta os fios da bomba na ordem cronológica das pistas. Dois fios errados fazem a bomba explodir.

**A ordem importa:** ir ao jardim sem resolver o salão, ou à galeria sem resolver o jardim, custa 15 minutos.

**Sala secreta:** existe um porão que o jogo não explica. Quem prestar atenção nas pistas da fonte e da galeria descobre como abri-lo.

### Finais

| Final | Como chegar |
|---|---|
| **Muito bom** | encontrar o porão secreto e confrontar o culpado antes que ele acione o detonador |
| **Bom** | acusar o culpado com as provas certas e desarmar a bomba |
| **Médio** | desarmar a bomba, mas acusar a pessoa errada (ou com as provas erradas) |
| **Ruim** | errar dois fios ou deixar o relógio chegar à meia-noite |

<details>
<summary><b>Solução completa (spoiler)</b></summary>

- **Salão:** a charada ("Tenho cúpula, mas não sou igreja. Guardo o jantar, mas não sou geladeira.") aponta para a **campânula** do bufê.
- **Jardim:** o mentiroso é o **Edu**.
- **Galeria:** a ordem é 1847, 1869, 1888, 1901, 1912, 1925.
- **Porão:** depois da galeria, examine a **fonte** do jardim (1847 → ladrilhos 1, 8, 4, 7). A porta de cima do jardim passa a levar ao porão.
- **Culpado:** Otávio Prado. As provas são as pistas das **22h20, 22h40 e 22h50**.
- **Fios:** Chave de corda → Fonte → Moldura → Luva.

</details>

---

## Estruturas de dados e algoritmos

| Estrutura / algoritmo | Onde | Para quê |
|---|---|---|
| **Lista circular duplamente encadeada** | `include/sala.h`, `src/sala.c` | As salas formam um anel (`proxSala` / `antSala`): as portas levam à sala seguinte ou à anterior. O porão secreto é **inserido no anel** em tempo de jogo (`sala_inserir_depois`), entre o jardim e a galeria. Para liberar, o anel é aberto e percorrido até o fim (`salas_liberar`). |
| **Lista simplesmente encadeada ordenada** | `include/jogador.h`, `src/jogador.c` | Inventário / linha do tempo. Cada pista é inserida já na posição do seu horário. A acusação percorre a lista para conferir as provas marcadas, e a ordem dos fios segue a ordem da lista. |
| **Insertion sort** | `src/puzzles.c` | Puzzle da galeria: o jogador ordena os quadros pegando um e inserindo na posição certa (`quadros_mover`). A resposta é conferida contra uma cópia ordenada por `quadros_insertion_sort`. |
| **Fila** | `include/ui.h`, `src/ui.c`, `src/puzzles.c` | Falas dos diálogos e depoimentos dos garçons, mostrados na ordem em que entraram. |

---

## Organização do código

| Arquivo | O que tem |
|---|---|
| `src/main.c` | janela, renderização em Full HD (textura 1920x1080) e laço principal |
| `include/jogo.h`, `src/jogo.c` | máquina de estados (menu, explorar, diálogo, puzzle, acusação, bomba, pausa, fim), interações, desenho das salas e personagens, música |
| `include/sala.h`, `src/sala.c` | anel de salas, paredes, portas, móveis e objetos de cada cenário |
| `include/jogador.h`, `src/jogador.c` | Bruno e o inventário (lista ordenada por horário) |
| `include/jogador_anim.h`, `src/jogador_anim.c` | animação do Bruno (5 direções x 4 quadros) |
| `include/puzzles.h`, `src/puzzles.c` | puzzles do jardim e da galeria |
| `include/ui.h`, `src/ui.c` | caixa de diálogo, escolha entre opções e avisos na tela |
| `include/inventario_tela.h`, `src/inventario_tela.c` | tela da linha do tempo |
| `include/final.h`, `src/final.c` | acusação, fios da bomba e telas dos finais |
| `include/tela.h` | resolução lógica fixa (1280x720) |
| `assets/` | menu, cenários, sprites e música |

A tecla **F3** mostra as caixas de colisão. Ela foi usada para alinhar os obstáculos com os desenhos.

---

## Equipe

| Integrante | Contribuição |
|---|---|
| **Matheus Costa da Rocha** | núcleo do jogo (salas, movimento, colisão, estados), puzzles, integração das partes, menu e resolução Full HD |
| **Luis Filipe Alves Silva Santos** | inventário, telas genéricas (diálogo, escolha, avisos), linha do tempo, acusação, fios da bomba e finais |
| **Amanda Campos Lima** | sprites, cenários e animação do personagem |
| **Leticia Camello Barros** | pausa e controle de som |

**Trilha sonora:** *The Midnight Alibi*.
