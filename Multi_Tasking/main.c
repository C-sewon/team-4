#define _CRT_SECURE_NO_WARNINGS
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>


#define WINDOW_W 1920
#define WINDOW_H 1080
#define RPS_ZONE_COUNT   3
#define RPS_PLAYER_SPEED 500.0f

//시간기록용 전역변수
#define RhythmT 1
#define RPST 20
#define JumpT 30
#define DodgeT 40

float gameOverTime = 0.0f;
bool gameOverTimeSaved = false;
bool globalGameOver=false;

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
#define RHYTHM_SPEED       300.0f
#define RHYTHM_HIT_X       180.0f
#define RHYTHM_JUDGE_WIDTH 80.0f
#define RHYTHM_TOLERANCE   80.0f

typedef struct {
    int type;   // 0=←, 1=↓, 2=↑, 3=→
    float x;    // X 좌표
    int active; // 1=활성, 0=비활성
} RhythmNote;

// ================ 수학 게임 (상단바) ================
#define MATH_TIME_LIMIT 7.0f
#define MATH_MAX_INPUT  10

// ================ 피하기 게임 (오른쪽 아래) ================
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

typedef enum { DINO_PLAYING, DINO_GAMEOVER } DinoState;

// ================ 전체 게임 상태 (타이틀/플레이) ================
typedef enum {
    PHASE_TITLE,   // 시작 화면
    PHASE_PLAYING  // 실제 멀티 게임 플레이
} GamePhase;

int main() 
{
    InitWindow(WINDOW_W, WINDOW_H, "Multitasking Game");
    SetTargetFPS(60);
    srand((unsigned)time(NULL));

    double startTime = GetTime();

    const int screenW = WINDOW_W;
    const int screenH = WINDOW_H;
    const int topBarHeight = 100;

    const int midX = screenW / 2;
    const int midY = screenH / 2 + 25;

    // ==== 각 게임 영역 (뷰포트) ====
    Rectangle vpRhythm = { 0, (float)topBarHeight, (float)midX, (float)(midY - topBarHeight) };        // 좌상
    Rectangle vpRPS    = { (float)midX, (float)topBarHeight, (float)(screenW - midX), (float)(midY - topBarHeight) }; // 우상
    Rectangle vpDino   = { 0, (float)midY, (float)midX, (float)(screenH - midY) };                     // 좌하
    Rectangle vpDodge  = { (float)midX, (float)midY, (float)(screenW - midX), (float)(screenH - midY) }; // 우하

    // ================ 수학 게임 상태 ================
    int   mathNum1 = rand() % 20 + 1;
    int   mathNum2 = rand() % 20 + 1;
    int   mathAnswer = mathNum1 + mathNum2;
    char  mathInput[MATH_MAX_INPUT] = { 0 };
    int   mathInputIndex = 0;
    float mathTimer = MATH_TIME_LIMIT;
    bool  mathGameOver = false;

    // ================ 피하기 게임 상태 ================
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

    // ================ 리듬 게임 상태 ================
    RhythmNote rhythmNotes[RHYTHM_MAX_NOTES];
    for (int i = 0; i < RHYTHM_MAX_NOTES; i++) rhythmNotes[i].active = 0;
    float rhythmSpawnTimer = 0.0f;
    float rhythmNextSpawn  = (float)(rand() % 1000) / 1000.0f + 0.5f; // 0.5~1.5
    int   rhythmGameOver   = 0;

    // ================ 가위바위보 게임 상태 ================
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

    int   RPSgameOver = 0;
    char  rpsResultText[32] = "";
    float rpsResultEndTime = 0.0f;
    const float rpsResultShowTime = 0.8f;

    // ================ 점프 게임 상태 (좌하단) ================
    float dinoW = vpDino.width;
    float dinoH = vpDino.height;

    DinoPlayer dinoPlayer;
    dinoPlayer.rect.x = 60;
    dinoPlayer.rect.y = dinoH - 70;
    dinoPlayer.rect.width  = 40;
    dinoPlayer.rect.height = 50;
    dinoPlayer.vy = 0.0f;
    dinoPlayer.onGround = true;

    float dinoGroundY = dinoH - 20;
    float dinoGravity = 1200.0f;
    float dinoJumpVel = -480.0f;

    DinoObstacle dinoObs[DINO_MAX_OBS];
    for (int i = 0; i < DINO_MAX_OBS; i++) dinoObs[i].active = false;

    float dinoSpawnTimer = 0.0f;
    float dinoNextSpawn  = 1.0f + (rand() % 120) / 60.0f;

    DinoState dinoState = DINO_PLAYING;
    float dinoGameSpeed = 300.0f;
    float dinoScore     = 0.0f;
    float dinoHighscore = 0.0f;

    // ================ 게임 시작 상태: 타이틀 화면 ================
    GamePhase phase = PHASE_TITLE;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        float elapsed = (float)(GetTime() - startTime); //게임시간
        float now = (float)GetTime(); //전체시간
        if(globalGameOver)
        {
            dt=0.0f;
        }


        if(globalGameOver)
        {
            if(IsKeyPressed(KEY_R))
            {
                globalGameOver=false;
                gameOverTimeSaved=false;

                startTime=GetTime();

                mathGameOver=false;
                mathTimer=MATH_TIME_LIMIT;
                mathInputIndex=0;
                mathInput[0]='\0';

                rhythmGameOver=0;
                for(int i=0;i<RHYTHM_MAX_NOTES;i++)
                {
                    rhythmNotes[i].active=false;
                }

                dodgeState=DODGE_PLAYING;
                dodgeScore=0;
                for(int i=0;i<MAX_BULLETS;i++)
                {
                    dodgeBullets[i].active=false;
                }

                dinoState=DINO_PLAYING;
                dinoPlayer.rect.y=dinoH-70;
                dinoPlayer.vy=0;
                dinoPlayer.onGround=true;
                for(int i=0;i<DINO_MAX_OBS;i++)
                {
                    dinoObs[i].active=false;
                }

                RPSgameOver=0;
                rpsComputerChoice=-1;
                rpsResponseActive=0;
                rpsPlayerPos.x=rpsW/2;

                
            }

            if(IsKeyPressed(KEY_ESCAPE))
            {
                CloseWindow();
                return 0;
            }
        }

        //if (IsKeyPressed(KEY_ESCAPE))
        //{
           // CloseWindow();
           // return 0;
        //}

        // ================ PHASE_TITLE : 시작 화면 ================
        if (phase == PHASE_TITLE) {
            // 시작 버튼 위치
            float btnWidth  = 220.0f;
            float btnHeight = 80.0f;
            Rectangle startBtn = {
                screenW / 2.0f - btnWidth / 2.0f,
                2.0f * screenH / 3.0f - btnHeight / 2.0f,
                btnWidth,
                btnHeight
            };

            Vector2 mouse = GetMousePosition();
            bool hover = CheckCollisionPointRec(mouse, startBtn);

            // 클릭 시 게임 시작
            if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                phase = PHASE_PLAYING;
                startTime = GetTime();   // 타이머를 "게임 시작 시점"으로 초기화
            }

            // 타이틀 화면 그리기
            BeginDrawing();
            // 어두운 빨간색 배경
            ClearBackground((Color){ 60, 0, 0, 255 });

            // 타이틀 텍스트 - 화면 위쪽 1/3 지점
            const char* title = "MultiTokTok";
            int titleFontSize = 100;
            int titleWidth = MeasureText(title, titleFontSize);
            int titleX = screenW / 2 - titleWidth / 2;
            int titleY = screenH / 3 - titleFontSize / 2;
            DrawText(title, titleX, titleY, titleFontSize, RAYWHITE);

            // 시작 버튼
            Color btnColor = hover ? (Color){ 200, 60, 60, 255 } : (Color){ 150, 30, 30, 255 };
            DrawRectangleRec(startBtn, btnColor);
            DrawRectangleLinesEx(startBtn, 3.0f, RAYWHITE);

            const char* startText = "Start";
            int startFontSize = 32;
            int startTextWidth = MeasureText(startText, startFontSize);
            int startTextX = (int)(startBtn.x + startBtn.width / 2 - startTextWidth / 2);
            int startTextY = (int)(startBtn.y + startBtn.height / 2 - startFontSize / 2);
            DrawText(startText, startTextX, startTextY, startFontSize, RAYWHITE);

            if(globalGameOver)
            {
                DrawRectangle(0,0,screenH,screenW,Fade(BLACK,0.7f));
                DrawText("GAME OVER",screenH/2-180,screenW/2-120,80,RED);
                DrawText("Press R to Restart",screenH/2-110,screenW/2+40,24,RAYWHITE);
                DrawText("Press ESC to Exit",screenH/2-110,screenW/2+80,24,RAYWHITE);
            }



            EndDrawing();
            // 타이틀 화면일 땐 아래 게임 로직/렌더링은 스킵
            continue;
        }

        // ================ 여기부터는 실제 게임 플레이 (기존 코드) ================

        // ================ 수학 게임 업데이트 ================
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

            if (mathTimer==0) {
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
        }/* else {
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
            */
        // ================ 피하기 게임 업데이트 ================
        if(elapsed>=DodgeT){
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
        }
    }/* else if (dodgeState == DODGE_GAMEOVER) {
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
            */

        // ================ 리듬 게임 업데이트 ================
        if(elapsed>=RhythmT){
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
        } 
    }
        /*else {
            // R 키로 리듬 게임 리셋
            if (IsKeyPressed(KEY_R)) {
                for (int i = 0; i < RHYTHM_MAX_NOTES; i++) rhythmNotes[i].active = 0;
                rhythmSpawnTimer = 0.0f;
                rhythmNextSpawn = (float)(rand() % 1000) / 1000.0f + 0.5f;
                rhythmGameOver = 0;
            }
        }*/

        // ================ 점프 게임 업데이트 ================
        /*if (dinoState == DINO_GAMEOVER) {
            if (IsKeyPressed(KEY_R)) {
                // ResetDinoGame()
                for (int i = 0; i < DINO_MAX_OBS; i++) dinoObs[i].active = false;
                dinoPlayer.rect.y = dinoH - 70;
                dinoPlayer.vy = 0.0f;
                dinoPlayer.onGround = true;
                dinoSpawnTimer = 0.0f;
                dinoNextSpawn = 1.0f + (rand() % 120) / 60.0f;
                dinoScore = 0.0f;
                dinoGameSpeed = 300.0f;
                dinoState = DINO_PLAYING;
            }
        } else*/
         if(elapsed>=JumpT){
        if (dinoState == DINO_PLAYING) {
            if ((IsKeyPressed(KEY_SPACE)) && dinoPlayer.onGround) {
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
                dinoNextSpawn = 0.6f + (rand() % 140) / 100.0f;

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


        // ================ 가위바위보 게임 업데이트 ================

        //리셋
       /* if (RPSgameOver && IsKeyPressed(KEY_R)) {
            RPSgameOver = 0;
            rpsComputerChoice = -1;
            rpsResponseActive = 0;
            float interval = rpsSpawnIntervalMin +
                ((float)rand() / (float)RAND_MAX) * (rpsSpawnIntervalMax - rpsSpawnIntervalMin);
            rpsNextSpawnTime = now + interval;
            rpsResultText[0] = '\0';
            rpsResultEndTime = 0.0f;
            rpsPlayerPos.x = rpsW / 2.0f;
        }*/

        if(elapsed>=RPST){
        if (!RPSgameOver) {
            float move = 0.0f;
            if (IsKeyDown(KEY_A)) move -= 1.0f;
            if (IsKeyDown(KEY_D)) move += 1.0f;
            rpsPlayerPos.x += move * RPS_PLAYER_SPEED * dt;
            if (rpsPlayerPos.x < rpsPlayerRadius) rpsPlayerPos.x = rpsPlayerRadius;
            if (rpsPlayerPos.x > rpsW - rpsPlayerRadius) rpsPlayerPos.x = rpsW - rpsPlayerRadius;
        }
    }else{
        rpsPlayerPos.x = rpsW / 2.0f;
    }

        int rpsPlayerZone = (int)(rpsPlayerPos.x / rpsZoneWidth);
        if (rpsPlayerZone < 0) rpsPlayerZone = 0;
        if (rpsPlayerZone >= RPS_ZONE_COUNT) rpsPlayerZone = RPS_ZONE_COUNT - 1;

        if(elapsed>=RPST){
        if (!RPSgameOver && !rpsResponseActive && now >= rpsNextSpawnTime) {
            rpsComputerChoice = rand() % 3;
            rpsResponseActive = 1;
            rpsResponseEndTime = now + rpsResponseDuration;
        }

        if (!RPSgameOver && rpsResponseActive && now >= rpsResponseEndTime) {
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
                RPSgameOver = 1;
            }
        }
    }

        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawRectangle(0, 0, screenW, topBarHeight, BLACK);
        DrawText("MultiTokTok", 20, 15, 20, WHITE);
        DrawText(
            TextFormat("%02d:%02d", (int)elapsed / 60, (int)elapsed % 60),
            screenW - 100, 15, 40, WHITE
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
            int mathFont = 40;
            int mathWidth = MeasureText(mathBarText, mathFont);
            int mathX = screenW / 2 - mathWidth / 2;
            DrawText(mathBarText, mathX, 35, mathFont, mathGameOver ? RED : WHITE);

            // if (mathGameOver) {
            //     const char* msg = "MATH GAME OVER";
            //     int w = MeasureText(msg, 18);
            //     DrawText(msg, screenW / 2 - w / 2, 32, 18, MAROON);
            // }
        }

        // ================ 구분선 ================
        DrawLine(midX, topBarHeight, midX, screenH, BLACK);
        DrawLine(0, midY, screenW, midY, BLACK);
        DrawLine(0, topBarHeight, screenW, topBarHeight, BLACK);
	
        // ================ 우하단 : 피하기 게임 ================
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

            if(elapsed>=DodgeT){
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
            if (dodgeState == DODGE_GAMEOVER) {
                const char* msg = "GAME OVER";
                int fw = MeasureText(msg, 32);
                DrawText(msg, (int)(dodgeW / 2 - fw / 2), (int)(dodgeH / 2 - 40), 32, RED);

                /*const char* msg2 = "Press R to restart";
                int fw2 = MeasureText(msg2, 18);
                DrawText(msg2, (int)(dodgeW / 2 - fw2 / 2), (int)(dodgeH / 2 + 4), 18, DARKGRAY);*/
            }
        }else {
                // 활성화 전 안내 메시지
                DrawText("Dodge Game (W/S to move)", 10, 10, 18, DARKGRAY);
                DrawText(TextFormat("ACTIVATE IN %.0f SEC", DodgeT - elapsed), (int)(dodgeW / 2 - 100), (int)(dodgeH / 2 - 10), 20, BLACK);
            }

            EndMode2D();
            EndScissorMode();
        }

        // ================ 구분선 ================
        DrawLine(midX, topBarHeight, midX, screenH, BLACK);
        DrawLine(0, midY, screenW, midY, BLACK);
        DrawLine(0, topBarHeight, screenW, topBarHeight, BLACK);

        // ================ 좌상단 : 리듬 게임 ================
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

            if(elapsed>=RhythmT){
            float lineY = vpRhythm.height / 2.0f;
            DrawLine(0, (int)lineY, (int)vpRhythm.width, (int)lineY, GRAY);

            DrawRectangle((int)RHYTHM_HIT_X, (int)(lineY - 50), (int)RHYTHM_JUDGE_WIDTH, 100, Fade(LIGHTGRAY, 0.4f));
            DrawRectangleLines((int)RHYTHM_HIT_X, (int)(lineY - 50), (int)RHYTHM_JUDGE_WIDTH, 100, DARKGRAY);
            DrawText("Rhythm(←/↓/↑/→)", 10, 10, 20, DARKGRAY);

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
                const char* msg = "GAME OVER";
                int w = MeasureText(msg, 20);
                DrawText(msg, (int)(vpRhythm.width / 2 - w / 2), (int)(vpRhythm.height / 2 - 10), 20, RED);
            }
        }else {
                // 활성화 전 안내 메시지
                DrawText("Rhythm Game", 10, 10, 20, DARKGRAY);
                DrawText(TextFormat("ACTIVATE IN %.0f SEC", RhythmT - elapsed), (int)(vpRhythm.width / 2 - 100), (int)(vpRhythm.height / 2 - 10), 20, BLACK);
            }

            EndMode2D();
            EndScissorMode();
        }

        //================ 가위바위보 게임 ================

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

            if(elapsed>=RPST){
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
                    if (!RPSgameOver) {
                        float toNext = rpsNextSpawnTime - now;
                        if (toNext < 0) toNext = 0;
                        char tbuf[32];
                        snprintf(tbuf, sizeof(tbuf), "Next: %.2f", toNext);
                        DrawText(tbuf, 10, 10, 20, DARKGRAY);
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

                if (rpsResultEndTime > now && !RPSgameOver) {
                    int fs = 40;
                    int tw = MeasureText(rpsResultText, fs);
                    DrawText(rpsResultText, (int)(rpsW / 2 - tw / 2), (int)(rpsH / 2 - 20), fs, BLUE);
                }

                if (RPSgameOver) {
                    DrawRectangle(0, 0, (int)rpsW, (int)rpsH, Fade(BLACK, 0.5f));
                    const char* goText = "GAME OVER";
                    int goFs = 64;
                    int goTw = MeasureText(goText, goFs);
                    DrawText(goText, (int)(rpsW / 2 - goTw / 2), (int)(rpsH / 2 - 40), goFs, RED);
                }
            }
            else {
                    // 활성화 전 안내 메시지
                    DrawText("RPS Game (A/D to move)", 10, 10, 20, DARKGRAY);
                    DrawText(TextFormat("ACTIVATE IN %.0f SEC", RPST - elapsed), (int)(rpsW / 2 - 100), (int)(rpsH / 2 - 10), 20, BLACK);
                }

            EndMode2D();
            EndScissorMode();
        }
        // ================ 점프 게임 ================
        {
            Camera2D cam = { 0 };
            cam.target = (Vector2){ 0.0f, 0.0f };
            cam.offset = (Vector2){ vpDino.x, vpDino.y };
            cam.zoom = 1.0f;
            cam.rotation = 0.0f;

            BeginScissorMode((int)vpDino.x, (int)vpDino.y,
                             (int)vpDino.width, (int)vpDino.height);
            BeginMode2D(cam);

            DrawRectangle(0, 0, (int)dinoW, (int)dinoH, RAYWHITE);

            if(elapsed>=JumpT){
            DrawText("Jump Game (SPACE/UP)", 10, 10, 20, DARKGRAY);

            DrawRectangle(0, (int)dinoGroundY, (int)dinoW, (int)(dinoH - dinoGroundY), LIGHTGRAY);
            DrawLine(0, (int)dinoGroundY, (int)dinoW, (int)dinoGroundY, DARKGRAY);

            if (dinoPlayer.onGround) {
                float t = (float)GetTime() * 12.0f;
                int bob = (int)(4.0f * (sin(t) > 0 ? 1 : -1));
                DrawRectangleRec(dinoPlayer.rect, MAROON);
                DrawRectangle((int)(dinoPlayer.rect.x + 5),
                              (int)(dinoPlayer.rect.y + dinoPlayer.rect.height - 6 + bob),
                              12, 6, BLACK);
                DrawRectangle((int)(dinoPlayer.rect.x + dinoPlayer.rect.width - 17),
                              (int)(dinoPlayer.rect.y + dinoPlayer.rect.height - 6 - bob),
                              12, 6, BLACK);
            } else {
                DrawRectangleRec(dinoPlayer.rect, MAROON);
            }

            for (int i = 0; i < DINO_MAX_OBS; i++) {
                if (!dinoObs[i].active) continue;
                DrawRectangleRec(dinoObs[i].rect, DARKGREEN);
            }

            

            if (dinoState == DINO_PLAYING) {
                DrawText("SPACE / UP = Jump", 12, 36, 16, DARKGRAY);
            } else if (dinoState == DINO_GAMEOVER) {
                DrawText("GAME OVER", (int)(dinoW / 2 - 80), (int)(dinoH / 2 - 50), 40, RED);
                /*DrawText("Press R to restart", (int)(dinoW / 2 - 100), (int)(dinoH / 2 + 10), 20, DARKGRAY);*/

            }
        }else {
                // 활성화 전 안내 메시지
                DrawText("Jump Game (SPACE/UP)", 10, 10, 20, DARKGRAY);
                DrawText(TextFormat("ACTIVATE IN %.0f SEC", JumpT - elapsed), (int)(dinoW / 2 - 100), (int)(dinoH / 2 - 10), 20, BLACK);
            }

            EndMode2D();
            EndScissorMode();
        }
        
    if (RPSgameOver || mathGameOver || rhythmGameOver || dodgeState == DODGE_GAMEOVER || dinoState == DINO_GAMEOVER) {
        RPSgameOver = 1;
        mathGameOver = 1;
        rhythmGameOver = 1;
        dodgeState = DODGE_GAMEOVER;
        dinoState = DINO_GAMEOVER;
    

    if (!gameOverTimeSaved) {
        gameOverTime = elapsed;      
        gameOverTimeSaved = true;
        globalGameOver=true;
    }
    }

if (RPSgameOver) {
    DrawRectangle(0, 0, screenW, screenH, Fade(BLACK, 0.5f));
    DrawText("GAME OVER",screenW/2-200,screenH/2-30,80,RED);
    DrawText("Press R to Restart",screenW/2-150,screenH/2+100,32,RAYWHITE);
    DrawText("Press ESC to Exit",screenW/2-140,screenH/2+150,32,RAYWHITE);

    char timeText[64];
    sprintf(timeText, "Time: %.2f s", gameOverTime);
    int tw = MeasureText(timeText, 32);
    DrawText(timeText, screenW / 2 - tw / 2, screenH / 2 + 50, 32, RAYWHITE);
}
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
