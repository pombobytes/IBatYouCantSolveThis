#include "raylib.h"

int main(void) {
    InitWindow(1280, 720, "I Bat You Can't Solve It");
    SetWindowPosition(100, 100);
    ToggleBorderlessWindowed();
    SetTargetFPS(60);

    const char *titulo = "I Bat You Can't Solve It";
    int tamanhoFonte = 40;

    while (!WindowShouldClose()){
        if(IsKeyPressed(KEY_F11)){
            ToggleBorderlessWindowed();
        }

        int larguraTela=GetScreenWidth();
        int alturaTela=GetScreenHeight();
        int larguraTexto=MeasureText(titulo, tamanhoFonte); 

        int x = (larguraTela - larguraTexto)/2;
        int y = (alturaTela - tamanhoFonte)/2;


        BeginDrawing();
            ClearBackground(BLACK);
            DrawText(titulo, x, y, tamanhoFonte, WHITE);   
        EndDrawing();
    }

    CloseWindow();
    return 0;
}