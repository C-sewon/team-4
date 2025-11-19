#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

#define LANE_COUNT   5
#define MAX_BULLETS  64

typedef struct Bullet {
    Rectangle rect;
    float vx;
    bool active;
} Bullet;

typedef enum {
    STATE_PLAYING,
    STATE_GAMEOVER
} GameState;

int main(void) {
    const int WINDOW_W = 1920;
    const int WINDOW_H = 1080;

    InitWindow(WINDOW_W, WINDOW_H, "Multitasking Game + Dodge Game");
    SetTargetFPS(60);

    double startTime = GetTime();

    // ---- Dodge game state ----
    Rectangle lanes[LANE_COUNT];
    int currentIndex = LANE_COUNT / 2; // start center

    Bullet bullets[MAX_BULLETS] = { 0 };
    float spawnTimer = 0.0f;
    float nextSpawn = 0.8f;
    GameState state = STATE_PLAYING;
    int score = 0;

    bool dodgeInitialized = false;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        double elapsed = GetTime() - startTime;

        // ESC to quit
        if (IsKeyPressed(KEY_ESCAPE)) break;

        int screenW = GetScreenWidth();
        int screenH = GetScreenHeight();

        // ----- 4분할 레이아웃 계산 -----
        int topBarHeight = 50;

        int midX = screenW / 2;
        int midY = screenH / 2 + 25; // 원래 코드 그대로

        // 오른쪽 아래 피하기 게임 뷰포트
        float dodgeX = (float)midX;
        float dodgeY = (float)midY;
        float dodgeW = (float)(screenW - midX);
        float dodgeH = (float)(screenH - midY);

        // 피하기 게임용 월드 사이즈 (로컬 좌표)
        float gameW = dodgeW;
        float gameH = dodgeH;

        // 한 번만 레인 초기화
        if (!dodgeInitialized) {
            float barWidth = 26.0f;
            float laneHeight = gameH / (float)LANE_COUNT;
            float startY = 0.0f;
            float barX = gameW / 2.0f - barWidth / 2.0f;

            for (int i = 0; i < LANE_COUNT; i++) {
                lanes[i].x = barX;
                lanes[i].y = startY + i * laneHeight;
                lanes[i].width  = barWidth;
                lanes[i].height = laneHeight - 4.0f; // small gap
            }

            currentIndex = LANE_COUNT / 2;
            for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
            spawnTimer = 0.0f;
            nextSpawn = 0.8f;
            score = 0;
            state = STATE_PLAYING;

            dodgeInitialized = true;
        }

        // ===================
        //   피하기 게임 로직
        // ===================
        if (state == STATE_PLAYING) {
            // 인덱스 이동 (W/S, 순환)
            if (IsKeyPressed(KEY_W)) {
                currentIndex--;
                if (currentIndex < 0) currentIndex = LANE_COUNT - 1;
            }
            if (IsKeyPressed(KEY_S)) {
                currentIndex++;
                if (currentIndex >= LANE_COUNT) currentIndex = 0;
            }

            // 탄막 스폰 타이머
            spawnTimer += dt;
            if (spawnTimer >= nextSpawn) {
                spawnTimer = 0.0f;
                nextSpawn = 0.5f + (float)GetRandomValue(0, 120) / 100.0f; // 0.5 ~ 1.7초 랜덤

                int lane = GetRandomValue(0, LANE_COUNT - 1);
                bool fromLeft = (GetRandomValue(0, 1) == 0);

                // 비활성 bullet 하나 찾아서 사용
                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (!bullets[i].active) {
                        bullets[i].active = true;

                        float h = lanes[lane].height * 0.7f;
                        bullets[i].rect.height = h;
                        bullets[i].rect.width  = 26.0f;
                        bullets[i].rect.y = lanes[lane].y + (lanes[lane].height - h) / 2.0f;

                        if (fromLeft) {
                            bullets[i].rect.x = -bullets[i].rect.width - 40.0f;
                            bullets[i].vx = 400.0f;
                        } else {
                            bullets[i].rect.x = gameW + 40.0f;
                            bullets[i].vx = -400.0f;
                        }
                        break;
                    }
                }
            }

            // 탄막 이동 & 충돌 처리
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (!bullets[i].active) continue;

                bullets[i].rect.x += bullets[i].vx * dt;

                // 중앙 인덱스 칸과 충돌하면 패배
                if (CheckCollisionRecs(bullets[i].rect, lanes[currentIndex])) {
                    state = STATE_GAMEOVER;
                }

                // 화면 바깥 완전히 나가면 소멸 + 점수 증가
                if (bullets[i].rect.x < -150.0f || bullets[i].rect.x > gameW + 150.0f) {
                    bullets[i].active = false;
                    if (state == STATE_PLAYING) score++;
                }
            }
        }
        else if (state == STATE_GAMEOVER) {
            if (IsKeyPressed(KEY_R)) {
                // 리셋
                for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
                spawnTimer = 0.0f;
                nextSpawn = 0.8f;
                currentIndex = LANE_COUNT / 2;
                score = 0;
                state = STATE_PLAYING;
            }
        }

        // ===================
        //       렌더링
        // ===================
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // ---- 상단 바 ----
        DrawRectangle(0, 0, screenW, topBarHeight, LIGHTGRAY);
        DrawText("Player_Name", 20, 15, 20, BLACK);
        DrawText(
            TextFormat("%02d:%02d", (int)elapsed / 60, (int)elapsed % 60),
            screenW - 100, 15, 20, BLACK
        );

        DrawText("5 + 3 = ?", screenW / 2 - 70, 15, 20, BLACK);
        DrawText("Ans: -",  screenW / 2 + 60, 15, 20, DARKGRAY);

        // ---- 구분선 ----
        DrawLine(screenW / 2, topBarHeight, screenW / 2, screenH, BLACK);
        DrawLine(0, midY, screenW, midY, BLACK);
        DrawLine(0, topBarHeight, screenW, topBarHeight, BLACK);

        // ---- 각 영역 제목 ----
        DrawText("Rhythm Game", 150, 150, 20, GRAY);   // 좌상
        DrawText("RPS Move",    700, 150, 20, GRAY);   // 우상
        DrawText("Jump Game",   150, 400, 20, GRAY);   // 좌하
        DrawText("Dodge Game",  700, 400, 20, GRAY);   // 우하 (라벨)

        // ===================
        //  오른쪽 하단 피하기 게임
        // ===================

        // 카메라: 월드 (0,0 ~ gameW, gameH)를 뷰포트 (dodgeX, dodgeY)에 매핑
        Camera2D cam = { 0 };
        cam.target = (Vector2){ 0.0f, 0.0f };
        cam.offset = (Vector2){ dodgeX + 100, dodgeY };
        cam.rotation = 0.0f;
        cam.zoom = 1.0f;

        // 이 영역 밖으로는 안 그리도록 Scissor
        BeginScissorMode((int)dodgeX, (int)dodgeY, (int)dodgeW, (int)dodgeH);
        BeginMode2D(cam);

        // 피하기 게임 배경
        DrawRectangle(0, 0, (int)gameW, (int)gameH, (Color){ 190, 210, 235, 255 });

        // 중앙 인덱스 바
        for (int i = 0; i < LANE_COUNT; i++) {
            Color fill = (i == currentIndex) ? BLUE : LIGHTGRAY;
            DrawRectangleRec(lanes[i], fill);
            DrawRectangleLinesEx(lanes[i], 2.0f, (Color){ 40, 40, 40, 255 });
        }

        // 탄막
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!bullets[i].active) continue;
            DrawRectangleRec(bullets[i].rect, (Color){ 20, 20, 20, 255 });
        }

        if (state == STATE_GAMEOVER) {
            const char* msg = "GAME OVER";
            int fw = MeasureText(msg, 32);
            DrawText(msg, (int)(gameW / 2 - fw / 2), (int)(gameH / 2 - 40), 32, RED);

            const char* msg2 = "Press R to restart";
            int fw2 = MeasureText(msg2, 18);
            DrawText(msg2, (int)(gameW / 2 - fw2 / 2), (int)(gameH / 2 + 4), 18, (Color){ 40, 40, 40, 255 });
        }

        EndMode2D();
        EndScissorMode();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
