#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// ------------------ 공통 ------------------
#define WINDOW_W 1920
#define WINDOW_H 1080


int main() {
    InitWindow(WINDOW_W, WINDOW_H, "Multitasking Game");
    SetTargetFPS(60);
    srand((unsigned)time(NULL));

    double startTime = GetTime();

    const int screenW = WINDOW_W;
    const int screenH = WINDOW_H;
    const int topBarHeight = 50;

    const int midX = screenW / 2;
    const int midY = screenH / 2 + 25;



    // ---- 각 게임 영역 (뷰포트) ----
    Rectangle vpRhythm = { 0, (float)topBarHeight, (float)midX, (float)(midY - topBarHeight) };        // 좌상
    Rectangle vpRPS    = { (float)midX, (float)topBarHeight, (float)(screenW - midX), (float)(midY - topBarHeight) }; // 우상
    Rectangle vpDino   = { 0, (float)midY, (float)midX, (float)(screenH - midY) };                     // 좌하
    Rectangle vpDodge  = { (float)midX, (float)midY, (float)(screenW - midX), (float)(screenH - midY) }; // 우하

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        double elapsed = GetTime() - startTime;



        if (IsKeyPressed(KEY_ESCAPE)) break;

        // =============================
        //          렌더링
        // =============================
        BeginDrawing();
        ClearBackground(RAYWHITE);

        EndMode2D();
        EndScissorMode();
        

        EndDrawing();
    }

    CloseWindow();
    return 0;
}