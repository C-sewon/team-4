#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// ------------------ 공통 ------------------
#define WINDOW_W 1920
#define WINDOW_H 1080

// =====================
//      리듬게임 코드
// =====================
#define RHYTHM_MAX_NOTES   20
#define RHYTHM_SPEED       300.0f
#define RHYTHM_HIT_X       180.0f
#define RHYTHM_JUDGE_WIDTH 80.0f
#define RHYTHM_TOLERANCE   80.0f

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

    // ---- 리듬게임 초기화 ----
    RhythmGame rhythmGame;
    InitRhythmGame(&rhythmGame);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        double elapsed = GetTime() - startTime;

        if (IsKeyPressed(KEY_ESCAPE)) break;

        // =============================
        //          리듬게임 업데이트
        // =============================
        UpdateRhythmGame(&rhythmGame, dt);

        // =============================
        //          렌더링
        // =============================
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // ---- 좌상단 리듬게임 ----
        DrawRhythmGame(&rhythmGame, vpRhythm);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
