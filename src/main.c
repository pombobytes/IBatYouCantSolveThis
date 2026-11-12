#include "raylib.h"
#include "jogo.h"

int main(void) {
    InitWindow(1280, 720, "I Bat You Can't Solve It");
    SetWindowPosition(100, 100);
    ToggleBorderlessWindowed();
    SetTargetFPS(60);
    
    Jogo jogo;
    jogo_iniciar(&jogo);


    while (!WindowShouldClose()){
        if(IsKeyPressed(KEY_F11)){
            ToggleBorderlessWindowed();
        }
        jogo_atualizar(&jogo);
        BeginDrawing();
            ClearBackground(BLACK);
            jogo_desenhar(&jogo);  
        EndDrawing();
    }

    CloseWindow();
    return 0;
}