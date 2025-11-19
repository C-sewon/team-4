#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// ------------------ 공통 ------------------
<<<<<<< HEAD
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
    InitWindow(WINDOW_W, WINDOW_H, "Multitasking Game");
=======
#define WINDOW_W 1000
#define WINDOW_H 1000

// =====================
//      리듬게임 코드
// =====================
#define RHYTHM_MAX_NOTES   20
#define RHYTHM_SPEED       300.0f
#define RHYTHM_HIT_X       180.0f
#define RHYTHM_JUDGE_WIDTH 80.0f
#define RHYTHM_TOLERANCE   80.0f
#define MATH_TIME_LIMIT 7.0f
#define MATH_MAX_INPUT  10

typedef struct {
    int type;   // 0=←,1=↓,2=↑,3=→
    float x;
    int active;
} RhythmNote;

typedef struct {
    RhythmNote notes[RHYTHM_MAX_NOTES];
    float spawnTimer;
    float nextSpawn;
    int gameOver;
} RhythmGame;

void InitRhythmGame(RhythmGame *rg) {
    for (int i = 0; i < RHYTHM_MAX_NOTES; i++) rg->notes[i].active = 0;
    rg->spawnTimer = 0.0f;
    rg->nextSpawn = (float)(rand() % 1000)/1000.0f + 0.5f;
    rg->gameOver = 0;
}

void UpdateRhythmGame(RhythmGame *rg, float dt) {
    if (!rg->gameOver) {
        rg->spawnTimer += dt;
        if (rg->spawnTimer > rg->nextSpawn) {
            for (int i = 0; i < RHYTHM_MAX_NOTES; i++) {
                if (!rg->notes[i].active) {
                    rg->notes[i].active = 1;
                    rg->notes[i].type = rand() % 4;
                    rg->notes[i].x = WINDOW_W/2 + 100.0f; // 좌상단 viewport width에 맞게
                    break;
                }
            }
            rg->spawnTimer = 0.0f;
            rg->nextSpawn = (float)(rand() % 1000)/1000.0f + 0.5f;
        }

        for (int i = 0; i < RHYTHM_MAX_NOTES; i++) {
            if (!rg->notes[i].active) continue;
            rg->notes[i].x -= RHYTHM_SPEED * dt;

            if (rg->notes[i].x < RHYTHM_HIT_X - RHYTHM_TOLERANCE) rg->gameOver = 1;
        }

        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_RIGHT)) {
            int keyType = -1;
            if (IsKeyPressed(KEY_LEFT)) keyType=0;
            else if (IsKeyPressed(KEY_DOWN)) keyType=1;
            else if (IsKeyPressed(KEY_UP)) keyType=2;
            else if (IsKeyPressed(KEY_RIGHT)) keyType=3;

            int hit=0;
            for (int i = 0; i < RHYTHM_MAX_NOTES; i++) {
                if (rg->notes[i].active && rg->notes[i].type == keyType &&
                    (rg->notes[i].x > RHYTHM_HIT_X - RHYTHM_TOLERANCE &&
                     rg->notes[i].x < RHYTHM_HIT_X + RHYTHM_JUDGE_WIDTH + RHYTHM_TOLERANCE)) {
                    rg->notes[i].active = 0;
                    hit = 1;
                    break;
                }
            }
            if (!hit) rg->gameOver = 1;
        }
    } else {
        if (IsKeyPressed(KEY_R)) {
            InitRhythmGame(rg);
        }
    }
}

void DrawRhythmGame(RhythmGame *rg, Rectangle vpRhythm) {
    Camera2D cam = {0};
    cam.target = (Vector2){0.0f,0.0f};
    cam.offset = (Vector2){vpRhythm.x,vpRhythm.y};
    cam.zoom = 1.0f;

    BeginScissorMode((int)vpRhythm.x,(int)vpRhythm.y,(int)vpRhythm.width,(int)vpRhythm.height);
    BeginMode2D(cam);

    DrawRectangle(0,0,(int)vpRhythm.width,(int)vpRhythm.height,(Color){240,240,255,255});
    float lineY = vpRhythm.height/2.0f;
    DrawLine(0,(int)lineY,(int)vpRhythm.width,(int)lineY,GRAY);

    DrawRectangle((int)RHYTHM_HIT_X,(int)(lineY-50),(int)RHYTHM_JUDGE_WIDTH,100,Fade(LIGHTGRAY,0.4f));
    DrawRectangleLines((int)RHYTHM_HIT_X,(int)(lineY-50),(int)RHYTHM_JUDGE_WIDTH,100,DARKGRAY);
    DrawText("Rhythm",10,10,20,DARKGRAY);

    for (int i = 0; i < RHYTHM_MAX_NOTES; i++) {
        if (!rg->notes[i].active) continue;
        Color c=BLACK; const char* arrow="";
        switch(rg->notes[i].type) {
            case 0: c=RED; arrow="<"; break;
            case 1: c=BLUE; arrow="v"; break;
            case 2: c=GREEN; arrow="^"; break;
            case 3: c=ORANGE; arrow=">"; break;
        }
        DrawText(arrow,(int)rg->notes[i].x,(int)(lineY-25),50,c);
    }

    if (rg->gameOver) {
        const char* msg="GAME OVER (R to restart)";
        int w=MeasureText(msg,20);
        DrawText(msg,(int)(vpRhythm.width/2 - w/2),(int)(vpRhythm.height/2 -10),20,RED);
    }

    EndMode2D();
    EndScissorMode();
}

// =====================
//       기존 main
// =====================
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

    // ---- 리듬게임 초기화 ----
    RhythmGame rhythmGame;
    InitRhythmGame(&rhythmGame);

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
        // =============================
        //          리듬게임 업데이트
        // =============================
        UpdateRhythmGame(&rhythmGame, dt);

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
        }

        EndMode2D();
        EndScissorMode();
    
        // ---- 좌상단 리듬게임 ----
        DrawRhythmGame(&rhythmGame, vpRhythm);
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
