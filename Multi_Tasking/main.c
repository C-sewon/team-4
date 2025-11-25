#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define WINDOW_W 1000
#define WINDOW_H 1000
#define RPS_ZONE_COUNT   3
#define RPS_PLAYER_SPEED 500.0f

typedef enum { RPS_SCISSORS = 0, RPS_ROCK = 1, RPS_PAPER = 2 } RPS;

const char* RpsToStr(int r) {
    switch (r) {
    case RPS_SCISSORS: return "Scissors";
    case RPS_ROCK:     return "Rock";
    case RPS_PAPER:    return "Paper";
    default:           return "?";
    }
}

int RpsBeats(int a, int b) {
    if (a == RPS_ROCK     && b == RPS_SCISSORS) return 1;
    if (a == RPS_SCISSORS && b == RPS_PAPER)    return 1;
    if (a == RPS_PAPER    && b == RPS_ROCK)     return 1;
    return 0;
}

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

    Rectangle vpRhythm = { 0, (float)topBarHeight, (float)midX, (float)(midY - topBarHeight) };        // 좌상
    Rectangle vpRPS    = { (float)midX, (float)topBarHeight, (float)(screenW - midX), (float)(midY - topBarHeight) }; // 우상
    Rectangle vpDino   = { 0, (float)midY, (float)midX, (float)(screenH - midY) };                     // 좌하
    Rectangle vpDodge  = { (float)midX, (float)midY, (float)(screenW - midX), (float)(screenH - midY) }; // 우하

    float rpsW = vpRPS.width;
    float rpsH = vpRPS.height;

    Rectangle rpsZones[RPS_ZONE_COUNT];
    int rpsZoneWidth  = (int)(rpsW / RPS_ZONE_COUNT);
    int rpsZoneHeight = 160;
    if (rpsZoneHeight > (int)(rpsH * 0.6f)) rpsZoneHeight = (int)(rpsH * 0.6f);

    for (int i = 0; i < RPS_ZONE_COUNT; i++) {
        rpsZones[i].x = (float)(i * rpsZoneWidth);
        rpsZones[i].y = rpsH - (float)rpsZoneHeight;
        rpsZones[i].width  = (float)rpsZoneWidth;
        rpsZones[i].height = (float)rpsZoneHeight;
    }

    float   rpsPlayerRadius = 32.0f;
    Vector2 rpsPlayerPos = { rpsW / 2.0f, rpsZones[0].y + rpsZones[0].height / 2.0f };

    int   rpsComputerChoice = -1;
    float rpsSpawnIntervalMin = 3.0f;
    float rpsSpawnIntervalMax = 10.0f;
    float rpsNextSpawnTime = (float)GetTime() +
        rpsSpawnIntervalMin +
        ((float)rand() / (float)RAND_MAX) * (rpsSpawnIntervalMax - rpsSpawnIntervalMin);

    int   rpsResponseActive = 0;
    float rpsResponseDuration = 4.0f;
    float rpsResponseEndTime = 0.0f;

    int   gameOver = 0;
    char  rpsResultText[32] = "";
    float rpsResultEndTime = 0.0f;
    const float rpsResultShowTime = 0.8f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        double elapsed = GetTime() - startTime;

        if (IsKeyPressed(KEY_ESCAPE)) break;

        float now = (float)GetTime();

        //리셋
        if (gameOver && IsKeyPressed(KEY_R)) {
            gameOver = 0;
            rpsComputerChoice = -1;
            rpsResponseActive = 0;
            float interval = rpsSpawnIntervalMin +
                ((float)rand() / (float)RAND_MAX) * (rpsSpawnIntervalMax - rpsSpawnIntervalMin);
            rpsNextSpawnTime = now + interval;
            rpsResultText[0] = '\0';
            rpsResultEndTime = 0.0f;
            rpsPlayerPos.x = rpsW / 2.0f;
        }

        if (!gameOver) {
            float move = 0.0f;
            if (IsKeyDown(KEY_A)) move -= 1.0f;
            if (IsKeyDown(KEY_D)) move += 1.0f;
            rpsPlayerPos.x += move * RPS_PLAYER_SPEED * dt;
            if (rpsPlayerPos.x < rpsPlayerRadius) rpsPlayerPos.x = rpsPlayerRadius;
            if (rpsPlayerPos.x > rpsW - rpsPlayerRadius) rpsPlayerPos.x = rpsW - rpsPlayerRadius;
        }

        int rpsPlayerZone = (int)(rpsPlayerPos.x / rpsZoneWidth);
        if (rpsPlayerZone < 0) rpsPlayerZone = 0;
        if (rpsPlayerZone >= RPS_ZONE_COUNT) rpsPlayerZone = RPS_ZONE_COUNT - 1;

        if (!gameOver && !rpsResponseActive && now >= rpsNextSpawnTime) {
            rpsComputerChoice = rand() % 3;
            rpsResponseActive = 1;
            rpsResponseEndTime = now + rpsResponseDuration;
        }

        if (!gameOver && rpsResponseActive && now >= rpsResponseEndTime) {
            int playerChoice = rpsPlayerZone;

            if (RpsBeats(playerChoice, rpsComputerChoice)) {
                snprintf(rpsResultText, sizeof(rpsResultText), "Success");
                rpsResultEndTime = now + rpsResultShowTime;

                float interval = rpsSpawnIntervalMin +
                    ((float)rand() / (float)RAND_MAX) * (rpsSpawnIntervalMax - rpsSpawnIntervalMin);
                rpsNextSpawnTime = now + interval;
                rpsResponseActive = 0;
                rpsComputerChoice = -1;
            } else {
                if (playerChoice == rpsComputerChoice) {
                    snprintf(rpsResultText, sizeof(rpsResultText), "Tie - Game Over");
                } else {
                    snprintf(rpsResultText, sizeof(rpsResultText), "Wrong - Game Over");
                }
                rpsResultEndTime = now + rpsResultShowTime;
                gameOver = 1;
            }
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawRectangle(0, 0, screenW, topBarHeight, LIGHTGRAY);
        DrawText("Player_Name", 20, 15, 20, BLACK);
        DrawText(
            TextFormat("%02d:%02d", (int)elapsed / 60, (int)elapsed % 60),
            screenW - 100, 15, 20, BLACK
        );

        {
            Camera2D cam = { 0 };
            cam.target = (Vector2){ 0.0f, 0.0f };
            cam.offset = (Vector2){ vpRPS.x, vpRPS.y };
            cam.zoom = 1.0f;
            cam.rotation = 0.0f;

            BeginScissorMode((int)vpRPS.x, (int)vpRPS.y,
                             (int)vpRPS.width, (int)vpRPS.height);
            BeginMode2D(cam);

            DrawRectangle(0, 0, (int)rpsW, (int)rpsH, (Color){ 245, 235, 235, 255 });

            if (rpsComputerChoice != -1) {
                const char* cstr = RpsToStr(rpsComputerChoice);
                int fontSize = 48;
                int textW = MeasureText(cstr, fontSize);
                DrawText(cstr, (int)(rpsW / 2 - textW / 2), 50, fontSize, RED);

                float remain = rpsResponseEndTime - now;
                if (remain < 0) remain = 0;
                char tbuf[32];
                snprintf(tbuf, sizeof(tbuf), "Time: %.2f", remain);
                DrawText(tbuf, 10, 10, 20, DARKGRAY);
            } else {
                if (!gameOver) {
                    float toNext = rpsNextSpawnTime - now;
                    if (toNext < 0) toNext = 0;
                    char tbuf[32];
                    snprintf(tbuf, sizeof(tbuf), "Next: %.2f", toNext);
                    DrawText(tbuf, 10, 10, 20, DARKGRAY);
                } else {
                    DrawText("No more rounds", 10, 40, 20, DARKGRAY);
                }
            }


            for (int i = 0; i < RPS_ZONE_COUNT; i++) {
                DrawRectangleRec(rpsZones[i], LIGHTGRAY);
                if (i == rpsPlayerZone) {
                    DrawRectangleRec(
                        (Rectangle){ rpsZones[i].x, rpsZones[i].y, rpsZones[i].width, rpsZones[i].height },
                        Fade(RED, 0.25f)
                    );
                }
                DrawRectangleLines((int)rpsZones[i].x, (int)rpsZones[i].y,
                                   (int)rpsZones[i].width, (int)rpsZones[i].height, GRAY);

                const char* label = RpsToStr(i);
                int fs = 28;
                int tw = MeasureText(label, fs);
                DrawText(label,
                         (int)(rpsZones[i].x + rpsZones[i].width / 2 - tw / 2),
                         (int)(rpsZones[i].y + 12),
                         fs, BLACK);
            }

            DrawCircleV(rpsPlayerPos, rpsPlayerRadius, DARKGREEN);
            DrawCircleLines((int)rpsPlayerPos.x, (int)rpsPlayerPos.y, (int)rpsPlayerRadius, BLACK);

            if (rpsResultEndTime > now && !gameOver) {
                int fs = 40;
                int tw = MeasureText(rpsResultText, fs);
                DrawText(rpsResultText, (int)(rpsW / 2 - tw / 2), (int)(rpsH / 2 - 20), fs, BLUE);
            }

            if (gameOver) {
                DrawRectangle(0, 0, (int)rpsW, (int)rpsH, Fade(BLACK, 0.5f));
                const char* goText = "GAME OVER";
                int goFs = 64;
                int goTw = MeasureText(goText, goFs);
                DrawText(goText, (int)(rpsW / 2 - goTw / 2), (int)(rpsH / 2 - 40), goFs, RED);

                char cause[128];
                const char* comp = (rpsComputerChoice != -1) ? RpsToStr(rpsComputerChoice) : "?";
                const char* play = RpsToStr(rpsPlayerZone);
                snprintf(cause, sizeof(cause), "You chose %s but computer had %s", play, comp);
                int csFs = 20;
                int csTw = MeasureText(cause, csFs);
                DrawText(cause, (int)(rpsW / 2 - csTw / 2), (int)(rpsH / 2 + 30), csFs, LIGHTGRAY);

                DrawText("Press R to Restart or ESC to Quit", (int)(rpsW / 2 - MeasureText("Press R to Restart or ESC to Quit", 20) / 2),
                         (int)(rpsH / 2 + 70), 20, LIGHTGRAY);
            }

            EndMode2D();
            EndScissorMode();
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
