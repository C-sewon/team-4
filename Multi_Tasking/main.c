#include "raylib.h"
#include <stdlib.h>
#include <time.h>

#define WINDOW_W 1000
#define WINDOW_H 1000

// ------------------ 리듬 게임 ------------------
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

int main() {
    InitWindow(WINDOW_W, WINDOW_H, "Rhythm Game Only");
    SetTargetFPS(60);
    srand((unsigned)time(NULL));

    // 좌상단 뷰포트 영역
    Rectangle vpRhythm = { 0, 50, WINDOW_W / 2.0f, WINDOW_H / 2.0f - 50 };

    // 리듬게임 상태 초기화
    RhythmNote rhythmNotes[RHYTHM_MAX_NOTES];
    for (int i = 0; i < RHYTHM_MAX_NOTES; i++) rhythmNotes[i].active = 0;

    float rhythmSpawnTimer = 0.0f;
    float rhythmNextSpawn  = (float)(rand() % 1000) / 1000.0f + 0.5f;
    int rhythmGameOver = 0;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // =============================
        //      리듬 게임 업데이트
        // =============================
        if (!rhythmGameOver) {
            rhythmSpawnTimer += dt;
            if (rhythmSpawnTimer > rhythmNextSpawn) {
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
            for (int i = 0; i < RHYTHM_MAX_NOTES; i++) {
                if (!rhythmNotes[i].active) continue;
                rhythmNotes[i].x -= RHYTHM_SPEED * dt;

                if (rhythmNotes[i].x < RHYTHM_HIT_X - RHYTHM_TOLERANCE) rhythmGameOver = 1;
            }

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
            if (IsKeyPressed(KEY_R)) {
                for (int i = 0; i < RHYTHM_MAX_NOTES; i++) rhythmNotes[i].active = 0;
                rhythmSpawnTimer = 0.0f;
                rhythmNextSpawn = (float)(rand() % 1000) / 1000.0f + 0.5f;
                rhythmGameOver = 0;
            }
        }

        // =============================
        //      렌더링
        // =============================
        BeginDrawing();
        ClearBackground(RAYWHITE);

        Camera2D cam = {0};
        cam.target = (Vector2){0.0f, 0.0f};
        cam.offset = (Vector2){ vpRhythm.x, vpRhythm.y };
        cam.zoom = 1.0f;

        BeginScissorMode((int)vpRhythm.x, (int)vpRhythm.y,
                         (int)vpRhythm.width, (int)vpRhythm.height);
        BeginMode2D(cam);

        DrawRectangle(0, 0, (int)vpRhythm.width, (int)vpRhythm.height, (Color){240,240,255,255});
        float lineY = vpRhythm.height / 2.0f;
        DrawLine(0, (int)lineY, (int)vpRhythm.width, (int)lineY, GRAY);

        DrawRectangle((int)RHYTHM_HIT_X, (int)(lineY - 50), (int)RHYTHM_JUDGE_WIDTH, 100, Fade(LIGHTGRAY,0.4f));
        DrawRectangleLines((int)RHYTHM_HIT_X, (int)(lineY - 50), (int)RHYTHM_JUDGE_WIDTH, 100, DARKGRAY);
        DrawText("Rhythm", 10, 10, 20, DARKGRAY);

        for (int i = 0; i < RHYTHM_MAX_NOTES; i++) {
            if (!rhythmNotes[i].active) continue;
            Color c = BLACK;
            const char* arrow = "";
            switch (rhythmNotes[i].type) {
                case 0: c=RED;    arrow="<"; break;
                case 1: c=BLUE;   arrow="v"; break;
                case 2: c=GREEN;  arrow="^"; break;
                case 3: c=ORANGE; arrow=">"; break;
            }
            DrawText(arrow, (int)rhythmNotes[i].x, (int)(lineY - 25), 50, c);
        }

        if (rhythmGameOver) {
            const char* msg = "GAME OVER (R to restart)";
            int w = MeasureText(msg, 20);
            DrawText(msg, (int)(vpRhythm.width/2 - w/2), (int)(vpRhythm.height/2 - 10), 20, RED);
        }

        EndMode2D();
        EndScissorMode();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
