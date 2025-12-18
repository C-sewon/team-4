#define _CRT_SECURE_NO_WARNINGS
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define WINDOW_W 1920
#define WINDOW_H 1080
#define RPS_ZONE_COUNT   3

// 시간 트리거
#define RhythmT 10
#define RPST    20
#define JumpT   30
#define DodgeT  40

float gameOverTime = 0.0f;
bool  gameOverTimeSaved = false;
bool  globalGameOver = false;

// ================= 난이도 스테이트 =================
typedef enum {
    DIFF_EASY,
    DIFF_HARD
} Difficulty;

Difficulty gDifficulty = DIFF_EASY;

// ================= 난이도 설정 묶음 =================
typedef struct {
    // 수학
    int   MATH_NUM;
    float MATH_TIME_LIMIT;

    // 공룡(점프)
    float DINO_SPAWN_MIN;
    float DINO_SPAWN_MAX;
    float DINO_JUMP_VEL;

    // 리듬
    float RHYTHM_SPEED;
    float RHYTHM_SPAWN_MIN;
    float RHYTHM_SPAWN_MAX;

    // 피하기
    float DODGE_BULLET_SPEED;
    float DODGE_SPAWN_MIN;
    float DODGE_SPAWN_MAX;

    // RPS
    float RPS_PLAYER_SPEED;
    float RPS_SPAWN_MIN;
    float RPS_SPAWN_MAX;
    float RPS_FIRST_SPAWN_TIME;   // 첫 등장까지 대기(초)
    float RPS_RESPONSE_DURATION;  // 대응 시간(초)
} DifficultyConfig;

//================ 가위바위보 게임(오른쪽 위) ================
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

// ================ 리듬 게임 (왼쪽 위) ================
#define RHYTHM_MAX_NOTES   20
#define RHYTHM_HIT_X       180.0f
#define RHYTHM_JUDGE_WIDTH 80.0f
#define RHYTHM_TOLERANCE   80.0f

typedef struct {
    int type;   // 0=←, 1=↓, 2=↑, 3=→
    float x;    // X 좌표
    int active; // 1=활성, 0=비활성
} RhythmNote;

// ================ 수학 게임 (상단바) ================
#define MATH_MAX_INPUT 3

// ================ 피하기 게임 (오른쪽 아래) ================
#define LANE_COUNT  5
#define MAX_BULLETS 64

typedef struct {
    Rectangle rect;
    float vx;
    bool active;
} Bullet;

typedef enum {
    DODGE_PLAYING,
    DODGE_GAMEOVER
} DodgeState;

// ================ 점프 게임 (왼쪽 아래) ================
#define DINO_MAX_OBS 8

typedef struct {
    Rectangle rect;
    float vy;
    bool onGround;
} DinoPlayer;

typedef struct {
    Rectangle rect;
    bool active;
} DinoObstacle;

typedef enum {
    DINO_PLAYING,
    DINO_GAMEOVER
} DinoState;

// ================ 전체 게임 상태 (타이틀/플레이) ================
typedef enum {
    PHASE_TITLE,
    PHASE_SELECT_DIFFICULTY,
    PHASE_PLAYING
} GamePhase;

int main(void)
{
    InitWindow(WINDOW_W, WINDOW_H, "Multitasking Game");
    SetTargetFPS(60);
    srand((unsigned)time(NULL));

    const int screenW = WINDOW_W;
    const int screenH = WINDOW_H;
    const int topBarHeight = 100;

    const int midX = screenW / 2;
    const int midY = screenH / 2 + 25;

    // ==== 각 게임 영역 (뷰포트) ====
    Rectangle vpRhythm = { 0, (float)topBarHeight, (float)midX, (float)(midY - topBarHeight) }; // 좌상
    Rectangle vpRPS    = { (float)midX, (float)topBarHeight, (float)(screenW - midX), (float)(midY - topBarHeight) }; // 우상
    Rectangle vpDino   = { 0, (float)midY, (float)midX, (float)(screenH - midY) }; // 좌하
    Rectangle vpDodge  = { (float)midX, (float)midY, (float)(screenW - midX), (float)(screenH - midY) }; // 우하

    // ================= 난이도 프리셋 =================
    DifficultyConfig easyCfg = {
        .MATH_NUM = 4,
        .MATH_TIME_LIMIT = 12.0f,

        .DINO_SPAWN_MIN = 2.0f,
        .DINO_SPAWN_MAX = 5.0f,
        .DINO_JUMP_VEL  = -600.0f,

        .RHYTHM_SPEED     = 175.0f,
        .RHYTHM_SPAWN_MIN = 1.0f,
        .RHYTHM_SPAWN_MAX = 4.0f,

        .DODGE_BULLET_SPEED = 200.0f,
        .DODGE_SPAWN_MIN    = 2.0f,
        .DODGE_SPAWN_MAX    = 4.0f,

        .RPS_PLAYER_SPEED     = 1000.0f,
        .RPS_SPAWN_MIN        = 5.0f,
        .RPS_SPAWN_MAX        = 10.0f,
        .RPS_FIRST_SPAWN_TIME = 8.0f,
        .RPS_RESPONSE_DURATION= 10.0f
    };

    DifficultyConfig hardCfg = {
        .MATH_NUM = 20,
        .MATH_TIME_LIMIT = 7.0f,

        .DINO_SPAWN_MIN = 0.6f,
        .DINO_SPAWN_MAX = 2.0f,
        .DINO_JUMP_VEL  = -500.0f,

        .RHYTHM_SPEED     = 300.0f,
        .RHYTHM_SPAWN_MIN = 0.5f,
        .RHYTHM_SPAWN_MAX = 2.0f,

        .DODGE_BULLET_SPEED = 400.0f,
        .DODGE_SPAWN_MIN    = 1.5f,
        .DODGE_SPAWN_MAX    = 3.0f,

        .RPS_PLAYER_SPEED     = 500.0f,
        .RPS_SPAWN_MIN        = 3.0f,
        .RPS_SPAWN_MAX        = 10.0f,
        .RPS_FIRST_SPAWN_TIME = 3.0f,
        .RPS_RESPONSE_DURATION= 8.0f
    };

    // 현재 적용 난이도(선택 후 cfg가 확정됨)
    DifficultyConfig cfg = easyCfg;

    // ================== 게임 타이머 기준 ==================
    double startTime = GetTime();

    // ================== 수학 게임 상태 ==================
    int   mathNum1 = 1;
    int   mathNum2 = 1;
    int   mathAnswer = 2;
    char  mathInput[MATH_MAX_INPUT] = { 0 };
    int   mathInputIndex = 0;
    float mathTimer = 0.0f;
    bool  mathGameOver = false;
    bool  mathDie = false;

    // ================== 피하기 게임 상태 ==================
    float dodgeW = vpDodge.width;
    float dodgeH = vpDodge.height;

    Rectangle dodgeLanes[LANE_COUNT];
    float dodgeBarWidth   = 26.0f;
    float dodgeLaneHeight = dodgeH / (float)LANE_COUNT;
    float dodgeStartY     = 0.0f;
    float dodgeBarX       = dodgeW / 2.0f - dodgeBarWidth / 2.0f;

    for (int i = 0; i < LANE_COUNT; i++) {
        dodgeLanes[i].x = dodgeBarX;
        dodgeLanes[i].y = dodgeStartY + i * dodgeLaneHeight;
        dodgeLanes[i].width  = dodgeBarWidth;
        dodgeLanes[i].height = dodgeLaneHeight - 4.0f;
    }

    int dodgeCurrentIndex = LANE_COUNT / 2;
    Bullet dodgeBullets[MAX_BULLETS] = { 0 };
    float dodgeSpawnTimer = 0.0f;
    float dodgeNextSpawn = 1.0f;
    DodgeState dodgeState = DODGE_PLAYING;
    int dodgeScore = 0;

    // ================== 리듬 게임 상태 ==================
    RhythmNote rhythmNotes[RHYTHM_MAX_NOTES];
    for (int i = 0; i < RHYTHM_MAX_NOTES; i++) rhythmNotes[i].active = 0;
    float rhythmSpawnTimer = 0.0f;
    float rhythmNextSpawn = 1.0f;
    int   rhythmGameOver = 0;

    // ================== RPS 게임 상태 ==================
    float rpsW = vpRPS.width;
    float rpsH = vpRPS.height;

    Rectangle rpsZones[RPS_ZONE_COUNT];
    int rpsZoneWidth  = (int)(rpsW / RPS_ZONE_COUNT);
    int rpsZoneHeight = 160;
    if (rpsZoneHeight > (int)(rpsH * 0.6f)) rpsZoneHeight = (int)(rpsH * 0.6f);

    for (int i = 0; i < RPS_ZONE_COUNT; i++) {
        rpsZones[i].x      = (float)(i * rpsZoneWidth);
        rpsZones[i].y      = rpsH - (float)rpsZoneHeight;
        rpsZones[i].width  = (float)rpsZoneWidth;
        rpsZones[i].height = (float)rpsZoneHeight;
    }

    float   rpsPlayerRadius = 32.0f;
    Vector2 rpsPlayerPos = { 0 };

    int   rpsComputerChoice = -1;
    float rpsSpawnIntervalMin = 3.0f;
    float rpsSpawnIntervalMax = 10.0f;
    float rpsNextSpawnTime = 8.0f;

    int   rpsResponseActive = 0;
    float rpsResponseDuration = 4.0f;
    float rpsResponseEndTime = 0.0f;

    int   RPSgameOver = 0;
    char  rpsResultText[32] = "";
    float rpsResultEndTime = 0.0f;
    const float rpsResultShowTime = 0.8f;

    // ================== 점프 게임 상태 ==================
    float dinoW = vpDino.width;
    float dinoH = vpDino.height;

    DinoPlayer dinoPlayer;
    DinoObstacle dinoObs[DINO_MAX_OBS];
    for (int i = 0; i < DINO_MAX_OBS; i++) dinoObs[i].active = false;

    float dinoGroundY = dinoH - 20;
    float dinoGravity = 1000.0f;
    float dinoJumpVel = -480.0f;

    float dinoSpawnTimer = 0.0f;
    float dinoNextSpawn = 1.0f;

    DinoState dinoState = DINO_PLAYING;
    float dinoGameSpeed = 300.0f;
    float dinoScore = 0.0f;
    float dinoHighscore = 0.0f;

    // ================== 게임 페이즈 ==================
    GamePhase phase = PHASE_TITLE;

    // ======= “초기화 블록” : 선택된 cfg 기준으로 싹 초기화 =======
    // (함수로 안 빼고, 필요할 때마다 동일 블록을 그대로 사용)
    {
        // 수학
        mathNum1 = rand() % cfg.MATH_NUM + 1;
        mathNum2 = rand() % cfg.MATH_NUM + 1;
        mathAnswer = mathNum1 + mathNum2;
        mathInputIndex = 0;
        mathInput[0] = '\0';
        mathTimer = cfg.MATH_TIME_LIMIT;
        mathGameOver = false;
        mathDie = false;

        // 리듬
        rhythmGameOver = 0;
        for (int i = 0; i < RHYTHM_MAX_NOTES; i++) rhythmNotes[i].active = 0;
        rhythmSpawnTimer = 0.0f;
        rhythmNextSpawn = cfg.RHYTHM_SPAWN_MIN +
            ((float)rand() / (float)RAND_MAX) * (cfg.RHYTHM_SPAWN_MAX - cfg.RHYTHM_SPAWN_MIN);

        // 피하기
        dodgeState = DODGE_PLAYING;
        dodgeScore = 0;
        dodgeCurrentIndex = LANE_COUNT / 2;
        for (int i = 0; i < MAX_BULLETS; i++) dodgeBullets[i].active = false;
        dodgeSpawnTimer = 0.0f;
        dodgeNextSpawn = cfg.DODGE_SPAWN_MIN +
            ((float)rand() / (float)RAND_MAX) * (cfg.DODGE_SPAWN_MAX - cfg.DODGE_SPAWN_MIN);

        // 점프
        dinoState = DINO_PLAYING;
        dinoPlayer.rect.x = 60;
        dinoPlayer.rect.y = dinoH - 70;
        dinoPlayer.rect.width  = 40;
        dinoPlayer.rect.height = 50;
        dinoPlayer.vy = 0.0f;
        dinoPlayer.onGround = true;

        dinoGameSpeed = 300.0f;
        dinoScore = 0.0f;

        for (int i = 0; i < DINO_MAX_OBS; i++) dinoObs[i].active = false;
        dinoSpawnTimer = 0.0f;
        dinoNextSpawn = cfg.DINO_SPAWN_MIN +
            ((float)rand() / (float)RAND_MAX) * (cfg.DINO_SPAWN_MAX - cfg.DINO_SPAWN_MIN);

        // RPS
        RPSgameOver = 0;
        rpsComputerChoice = -1;
        rpsResponseActive = 0;
        rpsPlayerPos = (Vector2){ rpsW / 2.0f, rpsZones[0].y + rpsZones[0].height / 2.0f };
        rpsSpawnIntervalMin = cfg.RPS_SPAWN_MIN;
        rpsSpawnIntervalMax = cfg.RPS_SPAWN_MAX;
        rpsNextSpawnTime = (float)GetTime() + cfg.RPS_FIRST_SPAWN_TIME;
        rpsResponseDuration = cfg.RPS_RESPONSE_DURATION;
        rpsResultText[0] = '\0';
        rpsResultEndTime = 0.0f;

        // 글로벌 게임오버
        globalGameOver = false;
        gameOverTimeSaved = false;
        gameOverTime = 0.0f;
    }

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        float now = (float)GetTime();

        // elapsed는 게임 시작(startTime) 기준
        static float elapsed = 0.0f;
        if (!globalGameOver) elapsed = (float)(GetTime() - startTime);
        if (globalGameOver) dt = 0.0f;

        // ================== GAME OVER 상태에서 입력 ==================
        if (globalGameOver) {
            if (IsKeyPressed(KEY_R)) {
                // 선택된 난이도 유지 -> cfg 유지
                // 다만 혹시 gDifficulty만 믿고 싶으면 여기서 cfg 재선택 가능:
                cfg = (gDifficulty == DIFF_EASY) ? easyCfg : hardCfg;

                startTime = GetTime();

                // ===== 초기화 블록(난이도 cfg 기준) =====
                {
                    // 수학
                    mathNum1 = rand() % cfg.MATH_NUM + 1;
                    mathNum2 = rand() % cfg.MATH_NUM + 1;
                    mathAnswer = mathNum1 + mathNum2;
                    mathInputIndex = 0;
                    mathInput[0] = '\0';
                    mathTimer = cfg.MATH_TIME_LIMIT;
                    mathGameOver = false;
                    mathDie = false;

                    // 리듬
                    rhythmGameOver = 0;
                    for (int i = 0; i < RHYTHM_MAX_NOTES; i++) rhythmNotes[i].active = 0;
                    rhythmSpawnTimer = 0.0f;
                    rhythmNextSpawn = cfg.RHYTHM_SPAWN_MIN +
                        ((float)rand() / (float)RAND_MAX) * (cfg.RHYTHM_SPAWN_MAX - cfg.RHYTHM_SPAWN_MIN);

                    // 피하기
                    dodgeState = DODGE_PLAYING;
                    dodgeScore = 0;
                    dodgeCurrentIndex = LANE_COUNT / 2;
                    for (int i = 0; i < MAX_BULLETS; i++) dodgeBullets[i].active = false;
                    dodgeSpawnTimer = 0.0f;
                    dodgeNextSpawn = cfg.DODGE_SPAWN_MIN +
                        ((float)rand() / (float)RAND_MAX) * (cfg.DODGE_SPAWN_MAX - cfg.DODGE_SPAWN_MIN);

                    // 점프
                    dinoState = DINO_PLAYING;
                    dinoPlayer.rect.x = 60;
                    dinoPlayer.rect.y = dinoH - 70;
                    dinoPlayer.rect.width = 40;
                    dinoPlayer.rect.height = 50;
                    dinoPlayer.vy = 0.0f;
                    dinoPlayer.onGround = true;

                    dinoGameSpeed = 300.0f;
                    dinoScore = 0.0f;

                    for (int i = 0; i < DINO_MAX_OBS; i++) dinoObs[i].active = false;
                    dinoSpawnTimer = 0.0f;
                    dinoNextSpawn = cfg.DINO_SPAWN_MIN +
                        ((float)rand() / (float)RAND_MAX) * (cfg.DINO_SPAWN_MAX - cfg.DINO_SPAWN_MIN);

                    // RPS
                    RPSgameOver = 0;
                    rpsComputerChoice = -1;
                    rpsResponseActive = 0;
                    rpsPlayerPos = (Vector2){ rpsW / 2.0f, rpsZones[0].y + rpsZones[0].height / 2.0f };
                    rpsSpawnIntervalMin = cfg.RPS_SPAWN_MIN;
                    rpsSpawnIntervalMax = cfg.RPS_SPAWN_MAX;
                    rpsNextSpawnTime = (float)GetTime() + cfg.RPS_FIRST_SPAWN_TIME;
                    rpsResponseDuration = cfg.RPS_RESPONSE_DURATION;
                    rpsResultText[0] = '\0';
                    rpsResultEndTime = 0.0f;

                    // 글로벌
                    globalGameOver = false;
                    gameOverTimeSaved = false;
                    gameOverTime = 0.0f;
                }
            }

            if (IsKeyPressed(KEY_ESCAPE)) {
                CloseWindow();
                return 0;
            }
        }

        // ================== PHASE_TITLE ==================
        if (phase == PHASE_TITLE) {
            float btnWidth = 220.0f;
            float btnHeight = 80.0f;
            Rectangle startBtn = {
                screenW / 2.0f - btnWidth / 2.0f,
                2.0f * screenH / 3.0f - btnHeight / 2.0f,
                btnWidth, btnHeight
            };

            Vector2 mouse = GetMousePosition();
            bool hover = CheckCollisionPointRec(mouse, startBtn);

            if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                phase = PHASE_SELECT_DIFFICULTY;
            }

            BeginDrawing();
            ClearBackground((Color){ 60, 0, 0, 255 });

            const char* title = "MultiTokTok";
            int titleFontSize = 100;
            int titleWidth = MeasureText(title, titleFontSize);
            DrawText(title, screenW / 2 - titleWidth / 2, screenH / 3 - titleFontSize / 2, titleFontSize, RAYWHITE);

            Color btnColor = hover ? (Color){ 200, 60, 60, 255 } : (Color){ 150, 30, 30, 255 };
            DrawRectangleRec(startBtn, btnColor);
            DrawRectangleLinesEx(startBtn, 3.0f, RAYWHITE);

            const char* startText = "Start";
            int startFontSize = 32;
            int startTextWidth = MeasureText(startText, startFontSize);
            DrawText(startText,
                (int)(startBtn.x + startBtn.width / 2 - startTextWidth / 2),
                (int)(startBtn.y + startBtn.height / 2 - startFontSize / 2),
                startFontSize, RAYWHITE);

            EndDrawing();
            continue;
        }

        // ================== PHASE_SELECT_DIFFICULTY ==================
        if (phase == PHASE_SELECT_DIFFICULTY) {
            float btnWidth = 260.0f;
            float btnHeight = 100.0f;

            Rectangle easyBtn = { screenW / 4.0f - btnWidth / 2.0f, screenH / 2.0f - btnHeight / 2.0f, btnWidth, btnHeight };
            Rectangle hardBtn = { 3.0f * screenW / 4.0f - btnWidth / 2.0f, screenH / 2.0f - btnHeight / 2.0f, btnWidth, btnHeight };

            Vector2 mouse = GetMousePosition();
            bool hoverEasy = CheckCollisionPointRec(mouse, easyBtn);
            bool hoverHard = CheckCollisionPointRec(mouse, hardBtn);

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (hoverEasy) {
                    gDifficulty = DIFF_EASY;
                    cfg = easyCfg;

                    startTime = GetTime();

                    // 난이도 기반 초기화
                    {
                        mathNum1 = rand() % cfg.MATH_NUM + 1;
                        mathNum2 = rand() % cfg.MATH_NUM + 1;
                        mathAnswer = mathNum1 + mathNum2;
                        mathInputIndex = 0;
                        mathInput[0] = '\0';
                        mathTimer = cfg.MATH_TIME_LIMIT;
                        mathGameOver = false;
                        mathDie = false;

                        rhythmGameOver = 0;
                        for (int i = 0; i < RHYTHM_MAX_NOTES; i++) rhythmNotes[i].active = 0;
                        rhythmSpawnTimer = 0.0f;
                        rhythmNextSpawn = cfg.RHYTHM_SPAWN_MIN +
                            ((float)rand() / (float)RAND_MAX) * (cfg.RHYTHM_SPAWN_MAX - cfg.RHYTHM_SPAWN_MIN);

                        dodgeState = DODGE_PLAYING;
                        dodgeScore = 0;
                        dodgeCurrentIndex = LANE_COUNT / 2;
                        for (int i = 0; i < MAX_BULLETS; i++) dodgeBullets[i].active = false;
                        dodgeSpawnTimer = 0.0f;
                        dodgeNextSpawn = cfg.DODGE_SPAWN_MIN +
                            ((float)rand() / (float)RAND_MAX) * (cfg.DODGE_SPAWN_MAX - cfg.DODGE_SPAWN_MIN);

                        dinoState = DINO_PLAYING;
                        dinoPlayer.rect.x = 60;
                        dinoPlayer.rect.y = dinoH - 70;
                        dinoPlayer.rect.width = 40;
                        dinoPlayer.rect.height = 50;
                        dinoPlayer.vy = 0.0f;
                        dinoPlayer.onGround = true;

                        dinoJumpVel = cfg.DINO_JUMP_VEL; // 난이도 점프력
                        dinoGameSpeed = 300.0f;
                        dinoScore = 0.0f;

                        for (int i = 0; i < DINO_MAX_OBS; i++) dinoObs[i].active = false;
                        dinoSpawnTimer = 0.0f;
                        dinoNextSpawn = cfg.DINO_SPAWN_MIN +
                            ((float)rand() / (float)RAND_MAX) * (cfg.DINO_SPAWN_MAX - cfg.DINO_SPAWN_MIN);

                        RPSgameOver = 0;
                        rpsComputerChoice = -1;
                        rpsResponseActive = 0;
                        rpsPlayerPos = (Vector2){ rpsW / 2.0f, rpsZones[0].y + rpsZones[0].height / 2.0f };
                        rpsSpawnIntervalMin = cfg.RPS_SPAWN_MIN;
                        rpsSpawnIntervalMax = cfg.RPS_SPAWN_MAX;
                        rpsNextSpawnTime = (float)GetTime() + cfg.RPS_FIRST_SPAWN_TIME;
                        rpsResponseDuration = cfg.RPS_RESPONSE_DURATION;

                        globalGameOver = false;
                        gameOverTimeSaved = false;
                        gameOverTime = 0.0f;
                    }

                    phase = PHASE_PLAYING;
                }
                else if (hoverHard) {
                    gDifficulty = DIFF_HARD;
                    cfg = hardCfg;

                    startTime = GetTime();

                    // 난이도 기반 초기화
                    {
                        mathNum1 = rand() % cfg.MATH_NUM + 1;
                        mathNum2 = rand() % cfg.MATH_NUM + 1;
                        mathAnswer = mathNum1 + mathNum2;
                        mathInputIndex = 0;
                        mathInput[0] = '\0';
                        mathTimer = cfg.MATH_TIME_LIMIT;
                        mathGameOver = false;
                        mathDie = false;

                        rhythmGameOver = 0;
                        for (int i = 0; i < RHYTHM_MAX_NOTES; i++) rhythmNotes[i].active = 0;
                        rhythmSpawnTimer = 0.0f;
                        rhythmNextSpawn = cfg.RHYTHM_SPAWN_MIN +
                            ((float)rand() / (float)RAND_MAX) * (cfg.RHYTHM_SPAWN_MAX - cfg.RHYTHM_SPAWN_MIN);

                        dodgeState = DODGE_PLAYING;
                        dodgeScore = 0;
                        dodgeCurrentIndex = LANE_COUNT / 2;
                        for (int i = 0; i < MAX_BULLETS; i++) dodgeBullets[i].active = false;
                        dodgeSpawnTimer = 0.0f;
                        dodgeNextSpawn = cfg.DODGE_SPAWN_MIN +
                            ((float)rand() / (float)RAND_MAX) * (cfg.DODGE_SPAWN_MAX - cfg.DODGE_SPAWN_MIN);

                        dinoState = DINO_PLAYING;
                        dinoPlayer.rect.x = 60;
                        dinoPlayer.rect.y = dinoH - 70;
                        dinoPlayer.rect.width = 40;
                        dinoPlayer.rect.height = 50;
                        dinoPlayer.vy = 0.0f;
                        dinoPlayer.onGround = true;

                        dinoJumpVel = cfg.DINO_JUMP_VEL; // 난이도 점프력
                        dinoGameSpeed = 300.0f;
                        dinoScore = 0.0f;

                        for (int i = 0; i < DINO_MAX_OBS; i++) dinoObs[i].active = false;
                        dinoSpawnTimer = 0.0f;
                        dinoNextSpawn = cfg.DINO_SPAWN_MIN +
                            ((float)rand() / (float)RAND_MAX) * (cfg.DINO_SPAWN_MAX - cfg.DINO_SPAWN_MIN);

                        RPSgameOver = 0;
                        rpsComputerChoice = -1;
                        rpsResponseActive = 0;
                        rpsPlayerPos = (Vector2){ rpsW / 2.0f, rpsZones[0].y + rpsZones[0].height / 2.0f };
                        rpsSpawnIntervalMin = cfg.RPS_SPAWN_MIN;
                        rpsSpawnIntervalMax = cfg.RPS_SPAWN_MAX;
                        rpsNextSpawnTime = (float)GetTime() + cfg.RPS_FIRST_SPAWN_TIME;
                        rpsResponseDuration = cfg.RPS_RESPONSE_DURATION;

                        globalGameOver = false;
                        gameOverTimeSaved = false;
                        gameOverTime = 0.0f;
                    }

                    phase = PHASE_PLAYING;
                }
            }

            BeginDrawing();
            ClearBackground((Color){ 20, 20, 30, 255 });

            const char* title = "Select Difficulty";
            int tf = 60;
            int tw = MeasureText(title, tf);
            DrawText(title, screenW / 2 - tw / 2, screenH / 4 - tf / 2, tf, RAYWHITE);

            DrawLine(screenW / 2, 0, screenW / 2, screenH, GRAY);

            Color easyColor = hoverEasy ? (Color){ 80, 200, 80, 255 } : (Color){ 40, 140, 40, 255 };
            Color hardColor = hoverHard ? (Color){ 220, 80, 80, 255 } : (Color){ 160, 40, 40, 255 };

            DrawRectangleRec(easyBtn, easyColor);
            DrawRectangleLinesEx(easyBtn, 3.0f, RAYWHITE);
            DrawText("EASY",
                (int)(easyBtn.x + easyBtn.width / 2 - MeasureText("EASY", 40) / 2),
                (int)(easyBtn.y + easyBtn.height / 2 - 40 / 2),
                40, RAYWHITE);

            DrawRectangleRec(hardBtn, hardColor);
            DrawRectangleLinesEx(hardBtn, 3.0f, RAYWHITE);
            DrawText("HARD",
                (int)(hardBtn.x + hardBtn.width / 2 - MeasureText("HARD", 40) / 2),
                (int)(hardBtn.y + hardBtn.height / 2 - 40 / 2),
                40, RAYWHITE);

            EndDrawing();
            continue;
        }

        // ================== 여기부터 실제 플레이 ==================

        // ----- 수학 업데이트 -----
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

            if (mathTimer == 0.0f) {
                if (mathInputIndex == 0) {
                    mathGameOver = true;
                    mathDie = true;
                }
                else {
                    int userAnswer = atoi(mathInput);
                    if (userAnswer == mathAnswer) {
                        mathNum1 = rand() % cfg.MATH_NUM + 1;
                        mathNum2 = rand() % cfg.MATH_NUM + 1;
                        mathAnswer = mathNum1 + mathNum2;
                        mathInputIndex = 0;
                        mathInput[0] = '\0';
                        mathTimer = cfg.MATH_TIME_LIMIT;
                        mathGameOver = false;
                        mathDie = false;
                    }
                    else {
                        mathGameOver = true;
                        mathDie = true;
                    }
                }
            }
        }

        // ----- 피하기 업데이트 -----
        if (elapsed >= DodgeT) {
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
                    dodgeNextSpawn =
                        cfg.DODGE_SPAWN_MIN +
                        ((float)rand() / (float)RAND_MAX) * (cfg.DODGE_SPAWN_MAX - cfg.DODGE_SPAWN_MIN);

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
                                dodgeBullets[i].vx =  cfg.DODGE_BULLET_SPEED;
                            }
                            else {
                                dodgeBullets[i].rect.x = dodgeW + 40.0f;
                                dodgeBullets[i].vx = -cfg.DODGE_BULLET_SPEED;
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
            }
        }

        // ----- 리듬 업데이트 -----
        if (elapsed >= RhythmT) {
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
                    rhythmNextSpawn =
                        cfg.RHYTHM_SPAWN_MIN +
                        ((float)rand() / (float)RAND_MAX) * (cfg.RHYTHM_SPAWN_MAX - cfg.RHYTHM_SPAWN_MIN);
                }

                for (int i = 0; i < RHYTHM_MAX_NOTES; i++) {
                    if (!rhythmNotes[i].active) continue;
                    rhythmNotes[i].x -= cfg.RHYTHM_SPEED * dt;

                    if (rhythmNotes[i].x < RHYTHM_HIT_X - RHYTHM_TOLERANCE) {
                        rhythmGameOver = 1;
                    }
                }

                if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_DOWN) ||
                    IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_RIGHT)) {

                    int keyType = -1;
                    if (IsKeyPressed(KEY_LEFT)) keyType = 0;
                    else if (IsKeyPressed(KEY_DOWN)) keyType = 1;
                    else if (IsKeyPressed(KEY_UP)) keyType = 2;
                    else if (IsKeyPressed(KEY_RIGHT)) keyType = 3;

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
            }
        }

        // ----- 점프 업데이트 -----
        if (elapsed >= JumpT) {
            if (dinoState == DINO_PLAYING) {
                if (IsKeyPressed(KEY_SPACE) && dinoPlayer.onGround) {
                    dinoPlayer.vy = dinoJumpVel;
                    dinoPlayer.onGround = false;
                }

                dinoPlayer.vy += dinoGravity * dt;
                dinoPlayer.rect.y += dinoPlayer.vy * dt;

                if (dinoPlayer.rect.y + dinoPlayer.rect.height >= dinoGroundY) {
                    dinoPlayer.rect.y = dinoGroundY - dinoPlayer.rect.height;
                    dinoPlayer.vy = 0.0f;
                    dinoPlayer.onGround = true;
                }

                dinoSpawnTimer += dt;
                if (dinoSpawnTimer >= dinoNextSpawn) {
                    dinoSpawnTimer = 0.0f;
                    dinoNextSpawn =
                        cfg.DINO_SPAWN_MIN +
                        ((float)rand() / (float)RAND_MAX) * (cfg.DINO_SPAWN_MAX - cfg.DINO_SPAWN_MIN);

                    for (int i = 0; i < DINO_MAX_OBS; i++) {
                        if (!dinoObs[i].active) {
                            dinoObs[i].active = true;
                            dinoObs[i].rect.width  = 20 + (rand() % 30);
                            dinoObs[i].rect.height = 30 + (rand() % 40);
                            dinoObs[i].rect.x = dinoW + 50;
                            dinoObs[i].rect.y = dinoGroundY - dinoObs[i].rect.height;
                            break;
                        }
                    }
                }

                for (int i = 0; i < DINO_MAX_OBS; i++) {
                    if (!dinoObs[i].active) continue;
                    dinoObs[i].rect.x -= dinoGameSpeed * dt;

                    if (dinoObs[i].rect.x + dinoObs[i].rect.width < -50) {
                        dinoObs[i].active = false;
                    }

                    if (CheckCollisionRecs(dinoObs[i].rect, dinoPlayer.rect)) {
                        dinoState = DINO_GAMEOVER;
                        if (dinoScore > dinoHighscore) dinoHighscore = dinoScore;
                        break;
                    }
                }

                dinoScore += dinoGameSpeed * dt * 0.01f;
                dinoGameSpeed += 5.0f * dt;
            }
        }

        // ----- RPS 업데이트 -----
        if (elapsed >= RPST) {
            if (!RPSgameOver) {
                float move = 0.0f;
                if (IsKeyDown(KEY_A)) move -= 1.0f;
                if (IsKeyDown(KEY_D)) move += 1.0f;
                rpsPlayerPos.x += move * cfg.RPS_PLAYER_SPEED * dt;
                if (rpsPlayerPos.x < rpsPlayerRadius) rpsPlayerPos.x = rpsPlayerRadius;
                if (rpsPlayerPos.x > rpsW - rpsPlayerRadius) rpsPlayerPos.x = rpsW - rpsPlayerRadius;
            }
        }
        else {
            rpsPlayerPos.x = rpsW / 2.0f;
        }

        int rpsPlayerZone = (int)(rpsPlayerPos.x / rpsZoneWidth);
        if (rpsPlayerZone < 0) rpsPlayerZone = 0;
        if (rpsPlayerZone >= RPS_ZONE_COUNT) rpsPlayerZone = RPS_ZONE_COUNT - 1;

        if (elapsed >= RPST) {
            if (!RPSgameOver && !rpsResponseActive && now >= rpsNextSpawnTime) {
                rpsComputerChoice = rand() % 3;
                rpsResponseActive = 1;
                rpsResponseEndTime = now + cfg.RPS_RESPONSE_DURATION;
            }

            if (!RPSgameOver && rpsResponseActive && now >= rpsResponseEndTime) {
                int playerChoice = rpsPlayerZone;

                if (RpsBeats(playerChoice, rpsComputerChoice)) {
                    snprintf(rpsResultText, sizeof(rpsResultText), "Success");
                    rpsResultEndTime = now + rpsResultShowTime;

                    float interval = cfg.RPS_SPAWN_MIN +
                        ((float)rand() / (float)RAND_MAX) * (cfg.RPS_SPAWN_MAX - cfg.RPS_SPAWN_MIN);
                    rpsNextSpawnTime = now + interval;

                    rpsResponseActive = 0;
                    rpsComputerChoice = -1;
                }
                else {
                    if (playerChoice == rpsComputerChoice) snprintf(rpsResultText, sizeof(rpsResultText), "Tie - Game Over");
                    else snprintf(rpsResultText, sizeof(rpsResultText), "Wrong - Game Over");

                    rpsResultEndTime = now + rpsResultShowTime;
                    RPSgameOver = 1;
                }
            }
        }

        // ================== 렌더 ==================
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawRectangle(0, 0, screenW, topBarHeight, BLACK);
        DrawText("MultiTokTok", 20, 15, 20, WHITE);
        DrawText(TextFormat("%02d:%02d", (int)elapsed / 60, (int)elapsed % 60),
            screenW - 140, 10, 50, WHITE);

        // 수학 바
        {
            char mathBarText[128];
            const char* ansStr = (mathInputIndex > 0) ? mathInput : "-";
            snprintf(mathBarText, sizeof(mathBarText),
                "%d + %d = ?   Ans: %s   (%.1fs)",
                mathNum1, mathNum2, ansStr, mathTimer);

            int font = 40;
            int w = MeasureText(mathBarText, font);
            int x = screenW / 2 - w / 2;
            DrawText(mathBarText, x, 35, font, mathDie ? RED : WHITE);
        }

        // 구분선
        DrawLine(midX, topBarHeight, midX, screenH, BLACK);
        DrawLine(0, midY, screenW, midY, BLACK);
        DrawLine(0, topBarHeight, screenW, topBarHeight, BLACK);

        // -------- 우하단 피하기 --------
        {
            Camera2D cam = { 0 };
            cam.target = (Vector2){ 0,0 };
            cam.offset = (Vector2){ vpDodge.x, vpDodge.y };
            cam.zoom = 1.0f;

            BeginScissorMode((int)vpDodge.x, (int)vpDodge.y, (int)vpDodge.width, (int)vpDodge.height);
            BeginMode2D(cam);

            DrawRectangle(0, 0, (int)dodgeW, (int)dodgeH, (Color){ 190, 210, 235, 255 });

            if (elapsed >= DodgeT) {
                for (int i = 0; i < LANE_COUNT; i++) {
                    Color fill = (i == dodgeCurrentIndex) ? BLUE : LIGHTGRAY;
                    DrawRectangleRec(dodgeLanes[i], fill);
                    DrawRectangleLinesEx(dodgeLanes[i], 2.0f, (Color){ 40,40,40,255 });
                }
                for (int i = 0; i < MAX_BULLETS; i++) if (dodgeBullets[i].active) DrawRectangleRec(dodgeBullets[i].rect, BLACK);
            }
            else {
                DrawText("Dodge Game (W/S to move)", 10, 10, 18, DARKGRAY);
                DrawText(TextFormat("ACTIVATE IN %.0f SEC", DodgeT - elapsed),
                    (int)(dodgeW / 2 - 100), (int)(dodgeH / 2 - 10), 20, BLACK);
            }

            EndMode2D();
            EndScissorMode();
        }

        // -------- 좌상단 리듬 --------
        {
            Camera2D cam = { 0 };
            cam.target = (Vector2){ 0,0 };
            cam.offset = (Vector2){ vpRhythm.x, vpRhythm.y };
            cam.zoom = 1.0f;

            BeginScissorMode((int)vpRhythm.x, (int)vpRhythm.y, (int)vpRhythm.width, (int)vpRhythm.height);
            BeginMode2D(cam);

            DrawRectangle(0, 0, (int)vpRhythm.width, (int)vpRhythm.height, (Color){ 240,240,255,255 });

            if (elapsed >= RhythmT) {
                float lineY = vpRhythm.height / 2.0f;
                DrawLine(0, (int)lineY, (int)vpRhythm.width, (int)lineY, GRAY);

                DrawRectangle((int)RHYTHM_HIT_X, (int)(lineY - 50), (int)RHYTHM_JUDGE_WIDTH, 100, Fade(LIGHTGRAY, 0.4f));
                DrawRectangleLines((int)RHYTHM_HIT_X, (int)(lineY - 50), (int)RHYTHM_JUDGE_WIDTH, 100, DARKGRAY);

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
            }
            else {
                DrawText("Rhythm Game(</v/^/>)", 10, 10, 20, DARKGRAY);
                DrawText(TextFormat("ACTIVATE IN %.0f SEC", RhythmT - elapsed),
                    (int)(vpRhythm.width / 2 - 100), (int)(vpRhythm.height / 2 - 10), 20, BLACK);
            }

            EndMode2D();
            EndScissorMode();
        }

        // -------- 우상단 RPS --------
        {
            Camera2D cam = { 0 };
            cam.target = (Vector2){ 0,0 };
            cam.offset = (Vector2){ vpRPS.x, vpRPS.y };
            cam.zoom = 1.0f;

            BeginScissorMode((int)vpRPS.x, (int)vpRPS.y, (int)vpRPS.width, (int)vpRPS.height);
            BeginMode2D(cam);

            DrawRectangle(0, 0, (int)rpsW, (int)rpsH, (Color){ 245,235,235,255 });

            if (elapsed >= RPST) {
                if (rpsComputerChoice != -1) {
                    const char* cstr = RpsToStr(rpsComputerChoice);
                    int fs = 48;
                    int w = MeasureText(cstr, fs);
                    DrawText(cstr, (int)(rpsW / 2 - w / 2), 50, fs, RED);

                    float remain = rpsResponseEndTime - now;
                    if (remain < 0) remain = 0;
                    DrawText(TextFormat("Time: %.2f", remain), 10, 10, 20, DARKGRAY);
                }
                else {
                    if (!RPSgameOver) {
                        float toNext = rpsNextSpawnTime - now;
                        if (toNext < 0) toNext = 0;
                        DrawText(TextFormat("Next: %.2f", toNext), 10, 10, 20, DARKGRAY);
                    }
                }

                for (int i = 0; i < RPS_ZONE_COUNT; i++) {
                    DrawRectangleRec(rpsZones[i], LIGHTGRAY);

                    if (i == rpsPlayerZone) {
                        DrawRectangleRec((Rectangle){ rpsZones[i].x, rpsZones[i].y, rpsZones[i].width, rpsZones[i].height },
                            Fade(RED, 0.25f));
                    }

                    DrawRectangleLines((int)rpsZones[i].x, (int)rpsZones[i].y, (int)rpsZones[i].width, (int)rpsZones[i].height, GRAY);

                    const char* label = RpsToStr(i);
                    int fs = 28;
                    int tw2 = MeasureText(label, fs);
                    DrawText(label,
                        (int)(rpsZones[i].x + rpsZones[i].width / 2 - tw2 / 2),
                        (int)(rpsZones[i].y + 12),
                        fs, BLACK);
                }

                DrawCircleV(rpsPlayerPos, rpsPlayerRadius, RED);
                DrawCircleLines((int)rpsPlayerPos.x, (int)rpsPlayerPos.y, (int)rpsPlayerRadius, BLACK);

                if (rpsResultEndTime > now && !RPSgameOver) {
                    int fs = 40;
                    int w = MeasureText(rpsResultText, fs);
                    DrawText(rpsResultText, (int)(rpsW / 2 - w / 2), (int)(rpsH / 2 - 20), fs, BLUE);
                }
            }
            else {
                DrawText("RPS Game (A/D to move)", 10, 10, 20, DARKGRAY);
                DrawText(TextFormat("ACTIVATE IN %.0f SEC", RPST - elapsed),
                    (int)(rpsW / 2 - 100), (int)(rpsH / 2 - 10), 20, BLACK);
            }

            EndMode2D();
            EndScissorMode();
        }

        // -------- 좌하단 점프 --------
        {
            Camera2D cam = { 0 };
            cam.target = (Vector2){ 0,0 };
            cam.offset = (Vector2){ vpDino.x, vpDino.y };
            cam.zoom = 1.0f;

            BeginScissorMode((int)vpDino.x, (int)vpDino.y, (int)vpDino.width, (int)vpDino.height);
            BeginMode2D(cam);

            DrawRectangle(0, 0, (int)dinoW, (int)dinoH, (Color){ 235,245,235,255 });

            if (elapsed >= JumpT) {
                DrawRectangle(0, (int)dinoGroundY, (int)dinoW, (int)(dinoH - dinoGroundY), LIGHTGRAY);
                DrawLine(0, (int)dinoGroundY, (int)dinoW, (int)dinoGroundY, DARKGRAY);

                if (dinoPlayer.onGround && dinoState == DINO_PLAYING) {
                    float t = (float)GetTime() * 12.0f;
                    int bob = (int)(4.0f * (sin(t) > 0 ? 1 : -1));
                    DrawRectangleRec(dinoPlayer.rect, DARKGREEN);
                    DrawRectangle((int)(dinoPlayer.rect.x + 5),
                        (int)(dinoPlayer.rect.y + dinoPlayer.rect.height - 6 + bob),
                        12, 6, BLACK);
                    DrawRectangle((int)(dinoPlayer.rect.x + dinoPlayer.rect.width - 17),
                        (int)(dinoPlayer.rect.y + dinoPlayer.rect.height - 6 - bob),
                        12, 6, BLACK);
                }
                else {
                    DrawRectangleRec(dinoPlayer.rect, DARKGREEN);
                }

                for (int i = 0; i < DINO_MAX_OBS; i++) if (dinoObs[i].active) DrawRectangleRec(dinoObs[i].rect, GRAY);
            }
            else {
                DrawText("Jump Game (SPACE)", 10, 10, 20, DARKGRAY);
                DrawText(TextFormat("ACTIVATE IN %.0f SEC", JumpT - elapsed),
                    (int)(dinoW / 2 - 100), (int)(dinoH / 2 - 10), 20, BLACK);
            }

            EndMode2D();
            EndScissorMode();
        }

        // ================== 글로벌 게임오버 체크 ==================
        if (RPSgameOver || mathGameOver || rhythmGameOver || dodgeState == DODGE_GAMEOVER || dinoState == DINO_GAMEOVER) {
            RPSgameOver = 1;
            mathGameOver = true;
            rhythmGameOver = 1;
            dodgeState = DODGE_GAMEOVER;
            dinoState = DINO_GAMEOVER;

            if (!gameOverTimeSaved) {
                gameOverTime = elapsed;
                gameOverTimeSaved = true;
                globalGameOver = true;
            }
        }

        // ================== GAME OVER UI ==================
        if (globalGameOver) {
            DrawRectangle(0, 0, screenW, screenH, Fade(BLACK, 0.5f));
            DrawText("GAME OVER", screenW / 2 - 240, screenH / 2 - 30, 80, RED);
            DrawText("Press R to Restart", screenW / 2 - 150, screenH / 2 + 100, 32, RAYWHITE);
            DrawText("Press ESC to Exit", screenW / 2 - 140, screenH / 2 + 150, 32, RAYWHITE);

            char timeText[64];
            sprintf(timeText, "Time: %.2f s", gameOverTime);
            int tw3 = MeasureText(timeText, 32);
            DrawText(timeText, screenW / 2 - tw3 / 2, screenH / 2 + 50, 32, RAYWHITE);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
