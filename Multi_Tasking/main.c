#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// ------------------ 공통 ------------------
#define WINDOW_W 1920
#define WINDOW_H 1080

// ------------------ 피하기 게임 (오른쪽 아래) ------------------
#define LANE_COUNT   5
#define MAX_BULLETS  64

typedef struct {
    Rectangle rect;
    float vx;
    bool active;
} Bullet;

typedef enum {
    DODGE_PLAYING,
    DODGE_GAMEOVER
} DodgeState;

int main() {
    InitWindow(WINDOW_W, WINDOW_H, "Multitasking Game (Math + Rhythm + RPS + Jump + Dodge)");
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

    // ------------------ 피하기 게임 상태 (우하단) ------------------
    float dodgeW = vpDodge.width;
    float dodgeH = vpDodge.height;

    Rectangle dodgeLanes[LANE_COUNT];
    float dodgeBarWidth  = 26.0f;
    float dodgeLaneHeight = dodgeH / (float)LANE_COUNT;
    float dodgeStartY    = 0.0f;
    float dodgeBarX      = dodgeW / 2.0f - dodgeBarWidth / 2.0f;

    for (int i = 0; i < LANE_COUNT; i++) {
        dodgeLanes[i].x = dodgeBarX;
        dodgeLanes[i].y = dodgeStartY + i * dodgeLaneHeight;
        dodgeLanes[i].width  = dodgeBarWidth;
        dodgeLanes[i].height = dodgeLaneHeight - 4.0f;
    }

    int dodgeCurrentIndex = LANE_COUNT / 2;
    Bullet dodgeBullets[MAX_BULLETS] = { 0 };
    float dodgeSpawnTimer = 0.0f;
    float dodgeNextSpawn  = 0.8f;
    DodgeState dodgeState = DODGE_PLAYING;
    int dodgeScore        = 0;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        double elapsed = GetTime() - startTime;

        if (IsKeyPressed(KEY_ESCAPE)) break;

        // =============================
        //          피하기 게임 업데이트
        // =============================
        if (dodgeState == DODGE_PLAYING) {
            if (IsKeyPressed(KEY_W)) {
                dodgeCurrentIndex--;
                if (dodgeCurrentIndex < 0) dodgeCurrentIndex = LANE_COUNT - 1;
            }
            if (IsKeyPressed(KEY_S)) {
                dodgeCurrentIndex++;
                if (dodgeCurrentIndex >= LANE_COUNT) dodgeCurrentIndex = 0;
            }

            dodgeSpawnTimer += dt;
            if (dodgeSpawnTimer >= dodgeNextSpawn) {
                dodgeSpawnTimer = 0.0f;
                dodgeNextSpawn = 0.5f + (float)GetRandomValue(0, 120) / 100.0f;

                int lane = GetRandomValue(0, LANE_COUNT - 1);
                bool fromLeft = (GetRandomValue(0, 1) == 0);

                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (!dodgeBullets[i].active) {
                        dodgeBullets[i].active = true;

                        float h = dodgeLanes[lane].height * 0.7f;
                        dodgeBullets[i].rect.height = h;
                        dodgeBullets[i].rect.width  = 26.0f;
                        dodgeBullets[i].rect.y = dodgeLanes[lane].y +
                            (dodgeLanes[lane].height - h) / 2.0f;

                        if (fromLeft) {
                            dodgeBullets[i].rect.x = -dodgeBullets[i].rect.width - 40.0f;
                            dodgeBullets[i].vx = 400.0f;
                        } else {
                            dodgeBullets[i].rect.x = dodgeW + 40.0f;
                            dodgeBullets[i].vx = -400.0f;
                        }
                        break;
                    }
                }
            }

            for (int i = 0; i < MAX_BULLETS; i++) {
                if (!dodgeBullets[i].active) continue;

                dodgeBullets[i].rect.x += dodgeBullets[i].vx * dt;

                if (CheckCollisionRecs(dodgeBullets[i].rect, dodgeLanes[dodgeCurrentIndex])) {
                    dodgeState = DODGE_GAMEOVER;
                }

                if (dodgeBullets[i].rect.x < -150.0f || dodgeBullets[i].rect.x > dodgeW + 150.0f) {
                    dodgeBullets[i].active = false;
                    if (dodgeState == DODGE_PLAYING) dodgeScore++;
                }
            }
        } else if (dodgeState == DODGE_GAMEOVER) {
            if (IsKeyPressed(KEY_R)) {
                // ResetDodgeGame()
                for (int i = 0; i < MAX_BULLETS; i++) dodgeBullets[i].active = false;
                dodgeSpawnTimer = 0.0f;
                dodgeNextSpawn = 0.8f;
                dodgeCurrentIndex = LANE_COUNT / 2;
                dodgeScore = 0;
                dodgeState = DODGE_PLAYING;
            }
        }

        // =============================
        //          렌더링
        // =============================
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // =========================================================
        //                    우하단 : 피하기 게임
        // =========================================================
        {
            Camera2D cam = { 0 };
            cam.target = (Vector2){ 0.0f, 0.0f };
            cam.offset = (Vector2){ vpDodge.x, vpDodge.y };
            cam.zoom = 1.0f;
            cam.rotation = 0.0f;

            BeginScissorMode((int)vpDodge.x, (int)vpDodge.y,
                             (int)vpDodge.width, (int)vpDodge.height);
            BeginMode2D(cam);

            DrawRectangle(0, 0, (int)dodgeW, (int)dodgeH, (Color){ 190, 210, 235, 255 });

            for (int i = 0; i < LANE_COUNT; i++) {
                Color fill = (i == dodgeCurrentIndex) ? BLUE : LIGHTGRAY;
                DrawRectangleRec(dodgeLanes[i], fill);
                DrawRectangleLinesEx(dodgeLanes[i], 2.0f, (Color){ 40, 40, 40, 255 });
            }

            for (int i = 0; i < MAX_BULLETS; i++) {
                if (!dodgeBullets[i].active) continue;
                DrawRectangleRec(dodgeBullets[i].rect, (Color){ 20, 20, 20, 255 });
            }

            DrawText("Dodge Game (W/S to move)", 10, 10, 18, DARKGRAY);
            DrawText(TextFormat("Score: %d", dodgeScore), 10, 34, 22, (Color){ 10, 10, 10, 255 });

            if (dodgeState == DODGE_GAMEOVER) {
                const char* msg = "GAME OVER";
                int fw = MeasureText(msg, 32);
                DrawText(msg, (int)(dodgeW / 2 - fw / 2), (int)(dodgeH / 2 - 40), 32, RED);

                const char* msg2 = "Press R to restart";
                int fw2 = MeasureText(msg2, 18);
                DrawText(msg2, (int)(dodgeW / 2 - fw2 / 2), (int)(dodgeH / 2 + 4), 18, DARKGRAY);
            }

            EndMode2D();
            EndScissorMode();
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}