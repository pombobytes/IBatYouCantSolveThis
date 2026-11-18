#include "raylib.h"
#include "tela.h"
#include "jogo.h"

int main(void) {
    InitWindow(TELA_LARGURA, TELA_ALTURA, "I Bat You Can't Solve It");
    SetExitKey(KEY_NULL);
    SetWindowPosition(100, 100);
    ToggleBorderlessWindowed();
    SetTargetFPS(60);

    // A lógica do jogo usa 1280x720 (tela.h), mas o desenho é feito numa textura
    // Full HD (1920x1080) com uma câmera de zoom 1.5: num monitor Full HD a imagem
    // sai 1:1, sem ampliação borrada. Em outras telas, a textura é ajustada.
    const float ZOOM_SAIDA = 1.5f;
    const int SAIDA_LARGURA = (int)(TELA_LARGURA * ZOOM_SAIDA);   // 1920
    const int SAIDA_ALTURA  = (int)(TELA_ALTURA  * ZOOM_SAIDA);   // 1080
    RenderTexture2D alvo = LoadRenderTexture(SAIDA_LARGURA, SAIDA_ALTURA);
    SetTextureFilter(alvo.texture, TEXTURE_FILTER_BILINEAR);    // suave se a tela não for Full HD
    Camera2D camera = { 0 };
    camera.zoom = ZOOM_SAIDA;

    jogo_carregar_recursos();   // imagens (menu etc.): precisa vir depois do InitWindow

    Jogo jogo;
    jogo_iniciar(&jogo);

    while (!WindowShouldClose()){
        if(IsKeyPressed(KEY_F11)){
            ToggleBorderlessWindowed();
        }

        // Escala da resolução fixa para a janela, mantendo a proporção (barras pretas se sobrar espaço)
        float escalaX = (float)GetScreenWidth()  / TELA_LARGURA;
        float escalaY = (float)GetScreenHeight() / TELA_ALTURA;
        float escala  = (escalaX < escalaY) ? escalaX : escalaY;
        Rectangle origem  = { 0, 0, (float)SAIDA_LARGURA, -(float)SAIDA_ALTURA }; // textura vem invertida
        Rectangle destino = {
            (GetScreenWidth()  - TELA_LARGURA * escala) / 2.0f,
            (GetScreenHeight() - TELA_ALTURA  * escala) / 2.0f,
            TELA_LARGURA * escala,
            TELA_ALTURA  * escala
        };

        // O mouse passa a responder em coordenadas da resolução fixa (1280x720),
        // então os botões do menu funcionam em qualquer tamanho de janela
        SetMouseOffset((int)-destino.x, (int)-destino.y);
        SetMouseScale(1.0f / escala, 1.0f / escala);

        jogo_atualizar(&jogo);

        // 1) desenha o jogo (coordenadas 1280x720) na textura Full HD, via câmera com zoom
        BeginTextureMode(alvo);
            ClearBackground(BLACK);
            BeginMode2D(camera);
                jogo_desenhar(&jogo);
            EndMode2D();
        EndTextureMode();

        // 2) amplia para a janela
        BeginDrawing();
            ClearBackground(BLACK);
            DrawTexturePro(alvo.texture, origem, destino, (Vector2){ 0, 0 }, 0.0f, WHITE);
        EndDrawing();
    }

    jogo_finalizar(&jogo);
    jogo_liberar_recursos();
    UnloadRenderTexture(alvo);
    CloseWindow();
    return 0;
}
