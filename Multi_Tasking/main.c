#include "raylib.h"
#include <stdio.h>

int main(void) {
    InitWindow(1000, 600, "Multitasking Game");
    SetTargetFPS(60);

    double startTime = GetTime();

    while (!WindowShouldClose()) {
        double elapsed = GetTime() - startTime;

        BeginDrawing();
        ClearBackground(RAYWHITE);

        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();

        // Top bar
        DrawRectangle(0, 0, screenW, 50, LIGHTGRAY);
        DrawText("Player_Name", 20, 15, 20, BLACK);
        DrawText(TextFormat("%02d:%02d", (int)elapsed / 60, (int)elapsed % 60),
                 screenW - 100, 15, 20, BLACK);

        DrawText("5 + 3 = ?", screenW / 2 - 70, 15, 20, BLACK);
        DrawText("Ans: -", screenW / 2 + 60, 15, 20, DARKGRAY);

        // Division lines
        DrawLine(screenW / 2, 50, screenW / 2, screenH, BLACK);
        DrawLine(0, (screenH / 2) + 25, screenW, (screenH / 2) + 25, BLACK);
        DrawLine(0, 50, screenW, 50, BLACK);

        DrawText("Rhythm Game", 150, 150, 20, GRAY);

        DrawText("RPS Move", 700, 150, 20, GRAY);

        DrawText("Jump Game", 150, 400, 20, GRAY);

        DrawText("Dodge Game", 700, 400, 20, GRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}