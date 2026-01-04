#include "raylib.h"
#include <stdio.h>

#define SCREEN_W 800
#define SCREEN_H 500

#define PADDLE_W 100
#define PADDLE_H 15
#define BALL_R   10

#define BRICK_ROWS 5
#define BRICK_COLS 10
#define BRICK_W 70
#define BRICK_H 20

// Game variables
int bricks[BRICK_ROWS][BRICK_COLS];
int score = 0, level = 1, lives = 3;
int highScore = 0;
int gameState = 0;  // 0=Welcome,1=Color,2=Play,3=GameOver,4=Scoreboard
char chatbotMsg[100];

Color ballColor = YELLOW;
Color paddleColor = SKYBLUE;

bool ghostBall = false;
float ghostTimer = 0.0f;


/* Load high score */
void LoadHighScore() {
    FILE *fp = fopen("highscore.txt", "r");
    if (fp) {
        fscanf(fp, "%d", &highScore);
        fclose(fp);
    }
}

/* Save high score */
void SaveHighScore() {
    FILE *fp = fopen("highscore.txt", "w");
    if (fp) {
        fprintf(fp, "%d", highScore);
        fclose(fp);
    }
}

/* Reset bricks */
void InitBricks() {
    for (int i = 0; i < BRICK_ROWS; i++)
        for (int j = 0; j < BRICK_COLS; j++)
            bricks[i][j] = 1;
}

/* Chatbot messages */
const char* GetChatbotMessage(int score) {
    if (score == 0) return "Let's go! Smash those bricks!";
    if (score < 50) return "Nice start, keep going!";
    if (score < 150) return "Great! You're improving!";
    if (score < 300) return "Amazing! You're playing well!";
    return "Unstoppable! Brick master!";
}

/* Scoreboard Screen */
void DrawScoreboardScreen() {

    DrawText("SCOREBOARD", SCREEN_W/2 - 120, 40, 40, YELLOW);

    FILE *fp = fopen("scores.txt", "r");
    int scoreList[50], count = 0;

    if (fp) {
        while (fscanf(fp, "%d", &scoreList[count]) != EOF && count < 50)
            count++;
        fclose(fp);
    }

    // Sort scores
    for (int i = 0; i < count - 1; i++)
        for (int j = i + 1; j < count; j++)
            if (scoreList[j] > scoreList[i]) {
                int t = scoreList[i];
                scoreList[i] = scoreList[j];
                scoreList[j] = t;
            }

    DrawText("Top 5 Scores:", SCREEN_W/2 - 80, 120, 30, WHITE);

    for (int i = 0; i < 5 && i < count; i++)
        DrawText(TextFormat("%d. %d", i + 1, scoreList[i]),
                 SCREEN_W/2 - 40, 180 + i * 40, 25, GREEN);

    DrawText("Press ESC to return", SCREEN_W/2 - 120, 430, 20, WHITE);
}


/* MAIN */
int main() {

    InitWindow(SCREEN_W, SCREEN_H, "Brick Breaker (Final Version)");
    SetTargetFPS(60);

    LoadHighScore();

    float paddleX = SCREEN_W/2 - PADDLE_W/2;
    float paddleY = SCREEN_H - 40;

    float bx = SCREEN_W/2, by = SCREEN_H/2;
    float sx = 4, sy = -4;

    InitBricks();

    while (!WindowShouldClose()) {

        /* ---- Update Logic ---- */
        if (gameState == 0) {  // Welcome
            if (IsKeyPressed(KEY_ENTER)) gameState = 1;
            if (IsKeyPressed(KEY_S)) gameState = 4;
        }

        else if (gameState == 1) {  // Color Select

            if (IsKeyPressed(KEY_ONE))  ballColor = YELLOW;
            if (IsKeyPressed(KEY_TWO))  ballColor = RED;
            if (IsKeyPressed(KEY_THREE)) ballColor = GREEN;

            if (IsKeyPressed(KEY_EIGHT)) paddleColor = SKYBLUE;
            if (IsKeyPressed(KEY_NINE))  paddleColor = ORANGE;
            if (IsKeyPressed(KEY_ZERO))  paddleColor = PINK;

            if (IsKeyPressed(KEY_ENTER)) {
                score = 0; level = 1; lives = 3;
                ghostBall = false;
                bx = SCREEN_W/2; by = SCREEN_H/2;
                sx = 4; sy = -4;
                InitBricks();
                gameState = 2;
            }
        }

        else if (gameState == 2) {  // Gameplay

            // Paddle move
            if (IsKeyDown(KEY_LEFT)) paddleX -= 6;
            if (IsKeyDown(KEY_RIGHT)) paddleX += 6;
            if (paddleX < 0) paddleX = 0;
            if (paddleX + PADDLE_W > SCREEN_W)
                paddleX = SCREEN_W - PADDLE_W;

            // Ball move
            bx += sx;
            by += sy;

            if (bx - BALL_R <= 0 || bx + BALL_R >= SCREEN_W) sx *= -1;
            if (by - BALL_R <= 0) sy *= -1;

            // Paddle bounce
            if (by + BALL_R >= paddleY &&
                bx >= paddleX && bx <= paddleX + PADDLE_W)
                sy *= -1;

            // Ghost countdown
            if (ghostBall) {
                ghostTimer -= GetFrameTime();
                if (ghostTimer <= 0) ghostBall = false;
            }

            // Brick collisions
            for (int i = 0; i < BRICK_ROWS; i++) {
                for (int j = 0; j < BRICK_COLS; j++) {
                    if (bricks[i][j] == 1) {
                        int x = 50 + j * (BRICK_W + 5);
                        int y = 50 + i * (BRICK_H + 5);

                        if (bx >= x && bx <= x + BRICK_W &&
                            by + BALL_R >= y && by - BALL_R <= y + BRICK_H) {

                            bricks[i][j] = 0;
                            score += 10;

                            if (GetRandomValue(1, 12) == 1) {
                                ghostBall = true;
                                ghostTimer = 5.0f;
                            }

                            if (!ghostBall) sy *= -1;
                        }
                    }
                }
            }

            // Level complete
            int bricksLeft = 0;
            for (int i = 0; i < BRICK_ROWS; i++)
                for (int j = 0; j < BRICK_COLS; j++)
                    bricksLeft += bricks[i][j];

            if (bricksLeft == 0) {
                level++;
                sx *= 1.1f; sy *= 1.1f;
                InitBricks();
            }

            // Ball fell
            if (by > SCREEN_H) {
                lives--;
                if (lives > 0) {
                    bx = SCREEN_W/2; by = SCREEN_H/2;
                    sx = 4; sy = -4;
                } else {

                    if (score > highScore) {
                        highScore = score;
                        sprintf(chatbotMsg, "WELL DONE! NEW HIGH SCORE!");
                    } else {
                        sprintf(chatbotMsg, "TRY AGAIN, YOU CAN DO IT!");
                    }

                    SaveHighScore();

                    FILE *sf = fopen("scores.txt", "a");
                    if (sf) { fprintf(sf, "%d\n", score); fclose(sf); }

                    gameState = 3;
                }
            }
        }

        else if (gameState == 3) {  // Game Over
            if (IsKeyPressed(KEY_R)) {
                score = 0; level = 1; lives = 3;
                bx = SCREEN_W/2; by = SCREEN_H/2;
                sx = 4; sy = -4;
                ghostBall = false;
                InitBricks();
                gameState = 2;
            }
        }

        else if (gameState == 4) {  // Scoreboard
            if (IsKeyPressed(KEY_ESCAPE))
                gameState = 0;
        }


        /* ---- DRAW ---- */
        BeginDrawing();
        ClearBackground(BLACK);

        if (gameState == 0) {  /* Draw Welcome */
            DrawText("WELCOME TO BRICK BREAKER", SCREEN_W/2 - 220, 150, 30, YELLOW);
            DrawText("Press ENTER to Choose Colors", SCREEN_W/2 - 180, 230, 20, GREEN);
            DrawText("Press S to View Scoreboard", SCREEN_W/2 - 160, 270, 20, SKYBLUE);
            DrawText(TextFormat("High Score: %d", highScore), SCREEN_W/2 - 100, 330, 20, ORANGE);
        }

        else if (gameState == 1) {  /* Draw Color Select */
            DrawText("SELECT COLORS", SCREEN_W/2 - 140, 70, 35, YELLOW);

            DrawText("BALL COLORS:", 200, 160, 22, WHITE);
            DrawText("1 - Yellow", 200, 200, 22, YELLOW);
            DrawText("2 - Red",    200, 230, 22, RED);
            DrawText("3 - Green",  200, 260, 22, GREEN);

            DrawText("PADDLE COLORS:", 450, 160, 22, WHITE);
            DrawText("8 - Blue",     450, 200, 22, SKYBLUE);
            DrawText("9 - Orange",   450, 230, 22, ORANGE);
            DrawText("0 - Pink",     450, 260, 22, PINK);

            DrawText("Press ENTER to Start Game", SCREEN_W/2 - 150, 380, 22, GREEN);
        }

        else if (gameState == 2) {  /* Draw Gameplay */

            for (int i = 0; i < BRICK_ROWS; i++)
                for (int j = 0; j < BRICK_COLS; j++)
                    if (bricks[i][j])
                        DrawRectangle(50 + j*(BRICK_W+5), 50 + i*(BRICK_H+5),
                                      BRICK_W, BRICK_H,
                                      (Color){200, 80+i*20, 150, 255});

            DrawRectangle(paddleX, paddleY, PADDLE_W, PADDLE_H, paddleColor);
            DrawCircle(bx, by, BALL_R, ghostBall ? WHITE : ballColor);

            DrawText(TextFormat("Score: %d", score), 20, 10, 20, WHITE);
            DrawText(TextFormat("Level: %d", level), 20, 35, 20, WHITE);
            DrawText(TextFormat("Lives: %d", lives), 20, 60, 20, RED);

            DrawText(GetChatbotMessage(score), 20, 90, 20, GREEN);
// Ghost mode: ball passes through bricks temporarily
            if (ghostBall)
                DrawText("GHOST MODE!", SCREEN_W/2 - 80, 10, 22, YELLOW);
        }

        else if (gameState == 3) {  /* Draw Game Over */
            DrawText("GAME OVER!", SCREEN_W/2 - 150, SCREEN_H/2 - 80, 40, RED);
            DrawText(chatbotMsg, SCREEN_W/2 - 200, SCREEN_H/2 - 30, 25, GREEN);
            DrawText("Press R to Restart", SCREEN_W/2 - 130, SCREEN_H/2 + 20, 20, WHITE);
        }

        else if (gameState == 4) {  /* Draw Scoreboard */
            ClearBackground(DARKBLUE);
            DrawScoreboardScreen();
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
