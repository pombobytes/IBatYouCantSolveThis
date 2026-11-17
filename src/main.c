#include "raylib.h"
#include "tela.h"
#include "jogo.h"

int main(void) {
    InitWindow(TELA_LARGURA, TELA_ALTURA, "I Bat You Can't Solve It");
    SetExitKey(KEY_NULL);
    SetWindowPosition(100, 100);
    ToggleBorderlessWindowed();
    SetTargetFPS(60);

    // O jogo é desenhado nesta textura (1280x720) e depois ampliado para a janela
    RenderTexture2D alvo = LoadRenderTexture(TELA_LARGURA, TELA_ALTURA);
    SetTextureFilter(alvo.texture, TEXTURE_FILTER_POINT);   // pixel art nítida

    Jogo jogo;
    jogo_iniciar(&jogo);

    while (!WindowShouldClose()){
        if(IsKeyPressed(KEY_F11)){
            ToggleBorderlessWindowed();
        }
        jogo_atualizar(&jogo);

        // 1) desenha o jogo na resolução fixa
        BeginTextureMode(alvo);
            ClearBackground(BLACK);
            jogo_desenhar(&jogo);
        EndTextureMode();

        // 2) amplia para a janela mantendo a proporção (barras pretas se sobrar espaço)
        float escalaX = (float)GetScreenWidth()  / TELA_LARGURA;
        float escalaY = (float)GetScreenHeight() / TELA_ALTURA;
        float escala  = (escalaX < escalaY) ? escalaX : escalaY;
        Rectangle origem  = { 0, 0, (float)TELA_LARGURA, -(float)TELA_ALTURA }; // textura vem invertida
        Rectangle destino = {
            (GetScreenWidth()  - TELA_LARGURA * escala) / 2.0f,
            (GetScreenHeight() - TELA_ALTURA  * escala) / 2.0f,
            TELA_LARGURA * escala,
            TELA_ALTURA  * escala
        };

        BeginDrawing();
            ClearBackground(BLACK);
            DrawTexturePro(alvo.texture, origem, destino, (Vector2){ 0, 0 }, 0.0f, WHITE);
        EndDrawing();
    }

    jogo_finalizar(&jogo);
    UnloadRenderTexture(alvo);
    CloseWindow();
    return 0;
}
