#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// ------------------ 공통 ------------------
#define WINDOW_W 1000
#define WINDOW_H 1000

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

       
        EndDrawing();
    }

    CloseWindow();
    return 0;
}