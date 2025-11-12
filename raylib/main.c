#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <raylib.h>

#define SCREEN_W 800
#define SCREEN_H 600
#define ZONE_COUNT 3
#define PLAYER_SPEED 500.0f

typedef enum { SCISSORS = 0, ROCK = 1, PAPER = 2 } RPS;

const char* rps_to_str(int r) {
    switch(r) {
        case SCISSORS: return "Scissors";
        case ROCK:     return "Rock";
        case PAPER:    return "Paper";
        default:       return "?";
    }
}

int beats(int a, int b) {
    if (a == ROCK && b == SCISSORS) return 1;
    if (a == SCISSORS && b == PAPER) return 1;
    if (a == PAPER && b == ROCK) return 1;
    return 0;
}

int main(void) {
    InitWindow(SCREEN_W, SCREEN_H, "RPS Game");
    SetTargetFPS(60);

    srand((unsigned)time(NULL));

    Rectangle zones[ZONE_COUNT];
    int zoneWidth = SCREEN_W / ZONE_COUNT;
    int zoneHeight = 160;
    for (int i = 0; i < ZONE_COUNT; i++) {
        zones[i].x = i * zoneWidth;
        zones[i].y = SCREEN_H - zoneHeight;
        zones[i].width = zoneWidth;
        zones[i].height = zoneHeight;
    }

    float playerRadius = 32.0f;
    Vector2 playerPos = { SCREEN_W/2.0f, zones[0].y + zones[0].height/2.0f };

    int computerChoice = -1;
    float spawnIntervalMin = 3.0f;
    float spawnIntervalMax = 10.0f;
    float nextSpawnTime = (float)GetTime() + spawnIntervalMin + ((float)rand() / RAND_MAX) * (spawnIntervalMax - spawnIntervalMin);

    int responseActive = 0;
    float responseDuration = 4.0f;
    float responseEndTime = 0.0f;

    int score = 0;
    char resultText[32] = "";
    float resultEndTime = 0.0f;
    const float resultShowTime = 0.8f;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        float now = (float)GetTime();

        float move = 0.0f;
        if (IsKeyDown(KEY_A)) move -= 1.0f;
        if (IsKeyDown(KEY_D)) move += 1.0f;
        playerPos.x += move * PLAYER_SPEED * dt;
        if (playerPos.x < playerRadius) playerPos.x = playerRadius;
        if (playerPos.x > SCREEN_W - playerRadius) playerPos.x = SCREEN_W - playerRadius;

        int playerZone = (int)(playerPos.x / zoneWidth);
        if (playerZone < 0) playerZone = 0;
        if (playerZone >= ZONE_COUNT) playerZone = ZONE_COUNT - 1;

        if (!responseActive && now >= nextSpawnTime) {
            computerChoice = rand() % 3;
            responseActive = 1;
            responseEndTime = now + responseDuration;
        }

        if (responseActive && now >= responseEndTime) {
            int playerChoice = playerZone;

            if (beats(playerChoice, computerChoice)) {
                score += 1;
                snprintf(resultText, sizeof(resultText), "+1");
            } else if (playerChoice == computerChoice) {
                snprintf(resultText, sizeof(resultText), "0");
            } else {
                score -= 1;
                snprintf(resultText, sizeof(resultText), "-1");
            }
            resultEndTime = now + resultShowTime;

            float interval = spawnIntervalMin + ((float)rand() / RAND_MAX) * (spawnIntervalMax - spawnIntervalMin);
            nextSpawnTime = now + interval;
            responseActive = 0;
            computerChoice = -1;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (computerChoice != -1) {
            const char *cstr = rps_to_str(computerChoice);
            int fontSize = 64;
            int textW = MeasureText(cstr, fontSize);
            DrawText(cstr, SCREEN_W/2 - textW/2, 50, fontSize, RED);

            float remain = responseEndTime - now;
            if (remain < 0) remain = 0;
            char tbuf[32];
            snprintf(tbuf, sizeof(tbuf), "Time: %.2f", remain);
            DrawText(tbuf, 10, 10, 20, DARKGRAY);
        } else {
            float remainToNext = nextSpawnTime - now;
            if (remainToNext < 0) remainToNext = 0;
            char tbuf[64];
            snprintf(tbuf, sizeof(tbuf), "Next: %.2f", remainToNext);
            DrawText(tbuf, 10, 10, 20, DARKGRAY);
        }

        char scoreBuf[32];
        snprintf(scoreBuf, sizeof(scoreBuf), "Score: %d", score);
        DrawText(scoreBuf, SCREEN_W - MeasureText(scoreBuf, 20) - 10, 10, 20, BLACK);

        for (int i = 0; i < ZONE_COUNT; i++) {
            DrawRectangleRec(zones[i], LIGHTGRAY);

            if (i == playerZone) {
                DrawRectangleRec((Rectangle){zones[i].x, zones[i].y, zones[i].width, zones[i].height}, Fade(RED, 0.25f));
            }
            DrawRectangleLines((int)zones[i].x, (int)zones[i].y, (int)zones[i].width, (int)zones[i].height, GRAY);
            const char* label = rps_to_str(i);
            int fs = 28;
            int tw = MeasureText(label, fs);
            DrawText(label, zones[i].x + zones[i].width/2 - tw/2, zones[i].y + 12, fs, BLACK);
        }

        DrawCircleV(playerPos, playerRadius, DARKGREEN);
        DrawCircleLines((int)playerPos.x, (int)playerPos.y, (int)playerRadius, BLACK);

        if (resultEndTime > now) {
            int fs = 40;
            int tw = MeasureText(resultText, fs);
            DrawText(resultText, SCREEN_W/2 - tw/2, SCREEN_H/2 - 20, fs, BLUE);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}