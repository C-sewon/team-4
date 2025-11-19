#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// ------------------ 공통 ------------------
#define WINDOW_W 1000
#define WINDOW_H 1000


// ------------------ 리듬 게임 (왼쪽 위) ------------------
#define RHYTHM_MAX_NOTES   20
#define RHYTHM_SPEED       300.0f
#define RHYTHM_HIT_X       180.0f
#define RHYTHM_JUDGE_WIDTH 80.0f
#define RHYTHM_TOLERANCE   80.0f

typedef struct {
    int type;   // 0=←, 1=↓, 2=↑, 3=→
    float x;    // X 좌표
    int active; // 1=활성, 0=비활성
} RhythmNote;

// ------------------ 수학 게임 (상단바) ------------------
#define MATH_TIME_LIMIT 7.0f
#define MATH_MAX_INPUT  10

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

    // ------------------ 수학 게임 상태 ------------------
    int   mathNum1 = rand() % 20 + 1;
    int   mathNum2 = rand() % 20 + 1;
    int   mathAnswer = mathNum1 + mathNum2;
    char  mathInput[MATH_MAX_INPUT] = { 0 };
    int   mathInputIndex = 0;
    float mathTimer = MATH_TIME_LIMIT;
    bool  mathGameOver = false;

    // ------------------ 리듬 게임 상태 (좌상단) ------------------
    RhythmNote rhythmNotes[RHYTHM_MAX_NOTES];
    for (int i = 0; i < RHYTHM_MAX_NOTES; i++) rhythmNotes[i].active = 0;
    float rhythmSpawnTimer = 0.0f;
    float rhythmNextSpawn  = (float)(rand() % 1000) / 1000.0f + 0.5f; // 0.5~1.5
    int   rhythmGameOver   = 0;


    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        double elapsed = GetTime() - startTime;

        if (IsKeyPressed(KEY_ESCAPE)) break;

        // =============================
        //          수학 게임 업데이트
        // =============================
        if (!mathGameOver) {
            mathTimer -= dt;
            if (mathTimer < 0.0f) mathTimer = 0.0f;

            int ch = GetCharPressed();
            while (ch > 0) {
                if (ch >= '0' && ch <= '9' && mathInputIndex < MATH_MAX_INPUT - 1) {
                    mathInput[mathInputIndex++] = (char)ch;
                    mathInput[mathInputIndex] = '\0';
                }
                ch = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE) && mathInputIndex > 0) {
                mathInput[--mathInputIndex] = '\0';
            }

            if (IsKeyPressed(KEY_ENTER)) {
                if (mathInputIndex == 0) {
                    mathGameOver = true;
                } else {
                    int userAnswer = atoi(mathInput);
                    if (userAnswer == mathAnswer) {
                        // ResetMathGame()
                        mathNum1 = rand() % 20 + 1;
                        mathNum2 = rand() % 20 + 1;
                        mathAnswer = mathNum1 + mathNum2;
                        mathInputIndex = 0;
                        mathInput[0] = '\0';
                        mathTimer = MATH_TIME_LIMIT;
                        mathGameOver = false;
                    } else {
                        mathGameOver = true;
                    }
                }
            }

            if (mathTimer <= 0.0f && !mathGameOver) {
                mathGameOver = true;
            }
        } else {
            // 수학 게임 GameOver 상태에서 ENTER로 새 문제
            if (IsKeyPressed(KEY_ENTER)) {
                // ResetMathGame()
                mathNum1 = rand() % 20 + 1;
                mathNum2 = rand() % 20 + 1;
                mathAnswer = mathNum1 + mathNum2;
                mathInputIndex = 0;
                mathInput[0] = '\0';
                mathTimer = MATH_TIME_LIMIT;
                mathGameOver = false;
            }
        }

        // =============================
        //          리듬 게임 업데이트
        // =============================
        if (!rhythmGameOver) {
            rhythmSpawnTimer += dt;
            if (rhythmSpawnTimer > rhythmNextSpawn) {
                // 노트 생성
                for (int i = 0; i < RHYTHM_MAX_NOTES; i++) {
                    if (!rhythmNotes[i].active) {
                        rhythmNotes[i].active = 1;
                        rhythmNotes[i].type = rand() % 4;
                        rhythmNotes[i].x = vpRhythm.width + 100.0f;
                        break;
                    }
                }
                rhythmSpawnTimer = 0.0f;
                rhythmNextSpawn = (float)(rand() % 1000) / 1000.0f + 0.5f;
            }

            float lineY = vpRhythm.height / 2.0f;

            // 노트 이동
            for (int i = 0; i < RHYTHM_MAX_NOTES; i++) {
                if (!rhythmNotes[i].active) continue;
                rhythmNotes[i].x -= RHYTHM_SPEED * dt;

                if (rhythmNotes[i].x < RHYTHM_HIT_X - RHYTHM_TOLERANCE) {
                    rhythmGameOver = 1;
                }
            }

            // 입력 처리
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_DOWN) ||
                IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_RIGHT)) {

                int keyType = -1;
                if (IsKeyPressed(KEY_LEFT))      keyType = 0;
                else if (IsKeyPressed(KEY_DOWN)) keyType = 1;
                else if (IsKeyPressed(KEY_UP))   keyType = 2;
                else if (IsKeyPressed(KEY_RIGHT))keyType = 3;

                int hit = 0;
                for (int i = 0; i < RHYTHM_MAX_NOTES; i++) {
                    if (rhythmNotes[i].active && rhythmNotes[i].type == keyType &&
                        (rhythmNotes[i].x > RHYTHM_HIT_X - RHYTHM_TOLERANCE &&
                         rhythmNotes[i].x < RHYTHM_HIT_X + RHYTHM_JUDGE_WIDTH + RHYTHM_TOLERANCE)) {
                        rhythmNotes[i].active = 0;
                        hit = 1;
                        break;
                    }
                }
                if (!hit) rhythmGameOver = 1;
            }
        } else {
            // R 키로 리듬 게임 리셋
            if (IsKeyPressed(KEY_R)) {
                for (int i = 0; i < RHYTHM_MAX_NOTES; i++) rhythmNotes[i].active = 0;
                rhythmSpawnTimer = 0.0f;
                rhythmNextSpawn = (float)(rand() % 1000) / 1000.0f + 0.5f;
                rhythmGameOver = 0;
            }
        }

        

        // =============================
        //          렌더링
        // =============================
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // ---- 상단 바 (수학 게임 + 메인 타이머) ----
        DrawRectangle(0, 0, screenW, topBarHeight, LIGHTGRAY);
        DrawText("Player_Name", 20, 15, 20, BLACK);
        DrawText(
            TextFormat("%02d:%02d", (int)elapsed / 60, (int)elapsed % 60),
            screenW - 100, 15, 20, BLACK
        );

        // 수학 게임 표시 텍스트
        {
            char mathBarText[128];
            const char* ansStr = (mathInputIndex > 0) ? mathInput : "-";
            snprintf(
                mathBarText, sizeof(mathBarText),
                "%d + %d = ?   Ans: %s   (%.1fs)",
                mathNum1, mathNum2, ansStr, mathTimer
            );
            int mathFont = 20;
            int mathWidth = MeasureText(mathBarText, mathFont);
            int mathX = screenW / 2 - mathWidth / 2;
            DrawText(mathBarText, mathX, 15, mathFont, mathGameOver ? RED : BLACK);

            if (mathGameOver) {
                const char* msg = "MATH GAME OVER - Press Enter to reset";
                int w = MeasureText(msg, 18);
                DrawText(msg, screenW / 2 - w / 2, 32, 18, MAROON);
            }
        }

        // ---- 구분선 ----
        DrawLine(midX, topBarHeight, midX, screenH, BLACK);
        DrawLine(0, midY, screenW, midY, BLACK);
        DrawLine(0, topBarHeight, screenW, topBarHeight, BLACK);

        // =========================================================
        //                    좌상단 : 리듬 게임
        // =========================================================
        {
            Camera2D cam = { 0 };
            cam.target = (Vector2){ 0.0f, 0.0f };
            cam.offset = (Vector2){ vpRhythm.x, vpRhythm.y };
            cam.zoom = 1.0f;
            cam.rotation = 0.0f;

            BeginScissorMode((int)vpRhythm.x, (int)vpRhythm.y,
                             (int)vpRhythm.width, (int)vpRhythm.height);
            BeginMode2D(cam);

            DrawRectangle(0, 0, (int)vpRhythm.width, (int)vpRhythm.height, (Color){ 240, 240, 255, 255 });

            float lineY = vpRhythm.height / 2.0f;
            DrawLine(0, (int)lineY, (int)vpRhythm.width, (int)lineY, GRAY);

            DrawRectangle((int)RHYTHM_HIT_X, (int)(lineY - 50), (int)RHYTHM_JUDGE_WIDTH, 100, Fade(LIGHTGRAY, 0.4f));
            DrawRectangleLines((int)RHYTHM_HIT_X, (int)(lineY - 50), (int)RHYTHM_JUDGE_WIDTH, 100, DARKGRAY);
            DrawText("Rhythm", 10, 10, 20, DARKGRAY);

            for (int i = 0; i < RHYTHM_MAX_NOTES; i++) {
                if (!rhythmNotes[i].active) continue;
                Color c = BLACK;
                const char* arrow = "";

                switch (rhythmNotes[i].type) {
                case 0: c = RED;    arrow = "<"; break;
                case 1: c = BLUE;   arrow = "v"; break;
                case 2: c = GREEN;  arrow = "^"; break;
                case 3: c = ORANGE; arrow = ">"; break;
                }
                DrawText(arrow, (int)rhythmNotes[i].x, (int)(lineY - 25), 50, c);
            }

            if (rhythmGameOver) {
                const char* msg = "GAME OVER (R to restart)";
                int w = MeasureText(msg, 20);
                DrawText(msg, (int)(vpRhythm.width / 2 - w / 2), (int)(vpRhythm.height / 2 - 10), 20, RED);
            }

            EndMode2D();
            EndScissorMode();
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}