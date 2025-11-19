#include "raylib.h"
#include <stdio.h>

// -------------------------
//  분할 열거형
// -------------------------
typedef enum {
    QUAD_LT, // Left Top
    QUAD_RT, // Right Top
    QUAD_LB, // Left Bottom
    QUAD_RB  // Right Bottom
} QuadType;

// -------------------------
//  화면 분할 정보
// -------------------------
typedef struct GameLayout {
    Rectangle quad[4];  // QUAD_LT ~ QUAD_RB
} GameLayout;

// -------------------------
//  화면 4분할 생성 함수
// -------------------------
GameLayout MakeGameLayout(int screenW, int screenH, int topBarH)
{
    GameLayout layout;

    int gameAreaY = topBarH;
    int gameAreaH = screenH - topBarH;

    int halfW = screenW / 2;
    int halfH = gameAreaH / 2;

    layout.quad[QUAD_LT] = (Rectangle){ 0,        gameAreaY,        halfW, halfH };
    layout.quad[QUAD_RT] = (Rectangle){ halfW,    gameAreaY,        halfW, halfH };
    layout.quad[QUAD_LB] = (Rectangle){ 0,        gameAreaY+halfH,  halfW, halfH };
    layout.quad[QUAD_RB] = (Rectangle){ halfW,    gameAreaY+halfH,  halfW, halfH };

    return layout;
}

// -------------------------
//  분할 내부 좌표 변환 함수
// -------------------------
// u, v는 0.0~1.0 : 분할 영역 상대 좌표
Vector2 GetPosInQuad(const GameLayout *layout, QuadType quad, float u, float v)
{
    Rectangle r = layout->quad[quad];
    Vector2 pos;
    pos.x = r.x + u * r.width;
    pos.y = r.y + v * r.height;
    return pos;
}


// -------------------------
//            메인
// -------------------------
static int screen_W = 1920;
static int screen_H = 1080;

int main(void) 
{
    InitWindow(screen_W, screen_H, "Multitasking Game");
    SetTargetFPS(60);

    double startTime = GetTime();
    const int topBarH = 50;

    while (!WindowShouldClose()) {

        double elapsed = GetTime() - startTime;

        // 매프레임마다 화면 분할 계산
        GameLayout layout = MakeGameLayout(screen_W, screen_H, topBarH);

        // -----------------------------------
        // 렌더링 시작
        // -----------------------------------
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // -------------------------
        //   상단 바
        // -------------------------
        DrawRectangle(0, 0, screen_W, topBarH, LIGHTGRAY);
        DrawText("Player_Name", 20, 15, 20, BLACK);
        DrawText(
            TextFormat("%02d:%02d", (int)elapsed / 60, (int)elapsed % 60),
            screen_W - 100, 15, 20, BLACK
        );

        // -------------------------
        //   분할 경계선 (디버그)
        // -------------------------
        DrawLine(screen_W / 2, topBarH, screen_W / 2, screen_H, BLACK);  // 세로선
        DrawLine(0, topBarH + (screen_H - topBarH)/2, screen_W, topBarH + (screen_H - topBarH)/2, BLACK); // 가로선

        // -------------------------
        //   예시 텍스트 출력 (각 분할의 중앙)
        // -------------------------
        int fontSize = 30;

        // Left Top
        {
            const char *txt = "Rhythm Game";
            int tw = MeasureText(txt, fontSize);
            Vector2 center = GetPosInQuad(&layout, QUAD_LT, 0.5f, 0.5f);
            DrawText(txt, center.x - tw/2, center.y - fontSize/2, fontSize, GRAY);
        }

        // Right Top
        {
            const char *txt = "RPS Move";
            int tw = MeasureText(txt, fontSize);
            Vector2 center = GetPosInQuad(&layout, QUAD_RT, 0.5f, 0.5f);
            DrawText(txt, center.x - tw/2, center.y - fontSize/2, fontSize, GRAY);
        }

        // Left Bottom
        {
            const char *txt = "Jump Game";
            int tw = MeasureText(txt, fontSize);
            Vector2 center = GetPosInQuad(&layout, QUAD_LB, 0.5f, 0.5f);
            DrawText(txt, center.x - tw/2, center.y - fontSize/2, fontSize, GRAY);
        }

        // Right Bottom
        {
            const char *txt = "Dodge Game";
            int tw = MeasureText(txt, fontSize);
            Vector2 center = GetPosInQuad(&layout, QUAD_RB, 0.5f, 0.5f);
            DrawText(txt, center.x - tw/2, center.y - fontSize/2, fontSize, GRAY);
        }

        EndDrawing();
        // -----------------------------------
        // 렌더링 끝
        // -----------------------------------
    }

    CloseWindow();
    return 0;
}
