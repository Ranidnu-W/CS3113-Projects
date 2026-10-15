/**
* Author: Deyon Wijewardana
* Assignment: Pong Clone
* Date due: 10/16/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/cs3113.h"

// Oscilloscope Pong: based on Tennis for Two (1958), which was played on
// a round oscilloscope screen. Each paddle slides around its half of the
// circle. The ball bounces off the paddles and the short walls at the very
// top and bottom. If it reaches the edge on your side, you lose.

// Global Constants
constexpr int SCREEN_WIDTH  = 1600 * 3 / 4,
              SCREEN_HEIGHT = 900 * 3 / 4,
              FPS           = 60,
              MAX_BALLS     = 5,
              TRAIL_LENGTH  = 18;

constexpr float BALL_SPEED       = 330.0f,
                BALL_RADIUS      = 9.0f,
                ARENA_RADIUS     = 307.5f,
                PADDLE_RADIUS    = 288.0f,  // how far the paddles are from the middle
                PADDLE_THICKNESS = 9.0f,
                PADDLE_SPEED     = 90.0f,   // degrees per second
                PADDLE_MAX_ANGLE = 54.0f,   // how far up/down a paddle can slide before it hits a wall
                PADDLE_HIT_RANGE = 60.0f,   // half the paddle's length, plus a bit for the ball
                WALL_HALF_ANGLE  = 27.0f,   // walls are drawn at 25, +2 so the ball still bounces off the ends
                DEG_TO_RAD       = 3.14159265f / 180.0f,
                TRAIL_INTERVAL   = 0.01f,
                TRAIL_ALPHA      = 140.0f;

constexpr char    BG_COLOUR[] = "#0E0F0F";
constexpr Vector2 ORIGIN         = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 },
                  SCREEN_SIZE    = { static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT) },
                  PADDLE_SIZE    = { 30.0f, 117.0f },
                  BALL_SIZE      = { 24.0f, 24.0f },
                  WIN_SIZE       = { 450.0f, 112.5f },
                  LEFT_INIT_POS  = { ORIGIN.x - PADDLE_RADIUS, ORIGIN.y },
                  RIGHT_INIT_POS = { ORIGIN.x + PADDLE_RADIUS, ORIGIN.y };

// where each ball starts and which way it goes first
constexpr Vector2 BALL_START_POS[MAX_BALLS] = {
    { ORIGIN.x, ORIGIN.y },
    { ORIGIN.x, ORIGIN.y - 45.0f },
    { ORIGIN.x, ORIGIN.y + 45.0f },
    { ORIGIN.x, ORIGIN.y - 90.0f },
    { ORIGIN.x, ORIGIN.y + 90.0f }
};
constexpr Vector2 BALL_START_MOVE[MAX_BALLS] = {
    { -1.0f,  0.25f },
    {  1.0f, -0.2f  },
    {  1.0f,  0.3f  },
    { -1.0f, -0.35f },
    { -1.0f,  0.15f }
};

constexpr char BACKGROUND_FP[] = "assets/background.png";
constexpr char PADDLE_FP[]     = "assets/paddle.png";
constexpr char BALL_FP[]       = "assets/ball.png";
constexpr char LEFT_WINS_FP[]  = "assets/left_wins.png";
constexpr char RIGHT_WINS_FP[] = "assets/right_wins.png";
constexpr char YOU_WIN_FP[]    = "assets/you_win.png";
constexpr char YOU_LOSE_FP[]   = "assets/you_lose.png";

// Global Variables
AppStatus gAppStatus     = RUNNING;
float     gPreviousTicks = 0.0f;

// paddle angles are in degrees from the middle of their side, positive = up
float gLeftAngle     = 0.0f,
      gLeftMovement  = 0.0f,
      gRightAngle    = 0.0f,
      gRightMovement = 0.0f;

Vector2 gLeftPosition  = LEFT_INIT_POS,
        gRightPosition = RIGHT_INIT_POS,
        gPaddleScale   = PADDLE_SIZE,

        gBallScale     = BALL_SIZE,
        gWinPosition   = ORIGIN,
        gWinScale      = WIN_SIZE,
        gBackgroundPosition = ORIGIN,
        gBackgroundScale    = SCREEN_SIZE;

// every ball that could be on screen, only the first gBallCount get used
Vector2 gBallPositions[MAX_BALLS];
Vector2 gBallMovements[MAX_BALLS];
int     gBallCount = 1;

// old ball positions for the phosphor glow trail
Vector2 gTrailPositions[MAX_BALLS][TRAIL_LENGTH];
float   gTrailTimer = 0.0f;

bool  gSinglePlayer   = false;
float gRightDirection = 1.0f;  // which way the right paddle goes in 1-player mode

int gWinner = 0;  // 0 = no one yet, 1 = left, 2 = right

Texture2D gBackgroundTexture;
Texture2D gPaddleTexture;
Texture2D gBallTexture;
Texture2D gLeftWinsTexture;
Texture2D gRightWinsTexture;
Texture2D gYouWinTexture;
Texture2D gYouLoseTexture;

// Function Declarations
void initialise();
void resetGame();
void processInput();
void update();
void render();
void shutdown();

// Function Definitions

/**
 * @brief Bounces a movement off a surface like a mirror.
 *
 * @param movement The movement to bounce (length 1)
 * @param normal The direction the surface is facing (length 1)
 */
void bounceOff(Vector2 *movement, const Vector2 *normal)
{
    // how much of the movement goes into the surface
    float into = movement->x * normal->x + movement->y * normal->y;

    // only bounce if it's actually going into it, otherwise it gets stuck
    if (into > 0.0f)
    {
        movement->x -= 2.0f * into * normal->x;
        movement->y -= 2.0f * into * normal->y;
    }
}

void renderObject(const Texture2D *texture, const Vector2 *position,
                  const Vector2 *scale, float angle, Color tint)
{
    // Whole texture (UV coordinates)
    Rectangle textureArea = {
        // top-left corner
        0.0f, 0.0f,

        // bottom-right corner (of texture)
        static_cast<float>(texture->width),
        static_cast<float>(texture->height)
    };

    // Destination rectangle – centred on position
    Rectangle destinationArea = {
        position->x,
        position->y,
        static_cast<float>(scale->x),
        static_cast<float>(scale->y)
    };

    // Origin inside the source texture (centre of the texture)
    Vector2 originOffset = {
        static_cast<float>(scale->x) / 2.0f,
        static_cast<float>(scale->y) / 2.0f
    };

    // Render the texture on screen
    DrawTexturePro(
        *texture,
        textureArea, destinationArea, originOffset,
        angle, tint
    );
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Oscilloscope Pong");

    gBackgroundTexture = LoadTexture(BACKGROUND_FP);
    gPaddleTexture     = LoadTexture(PADDLE_FP);
    gBallTexture       = LoadTexture(BALL_FP);
    gLeftWinsTexture   = LoadTexture(LEFT_WINS_FP);
    gRightWinsTexture  = LoadTexture(RIGHT_WINS_FP);
    gYouWinTexture     = LoadTexture(YOU_WIN_FP);
    gYouLoseTexture    = LoadTexture(YOU_LOSE_FP);

    resetGame();

    SetTargetFPS(FPS);
}

void resetGame()
{
    gWinner         = 0;
    gLeftAngle      = 0.0f;
    gRightAngle     = 0.0f;
    gRightDirection = 1.0f;

    for (int i = 0; i < MAX_BALLS; i++)
    {
        gBallPositions[i] = BALL_START_POS[i];
        gBallMovements[i] = BALL_START_MOVE[i];
        Normalise(&gBallMovements[i]);

        for (int j = 0; j < TRAIL_LENGTH; j++)
        {
            gTrailPositions[i][j] = BALL_START_POS[i];
        }
    }
}

void processInput()
{
    gLeftMovement  = 0.0f;
    gRightMovement = 0.0f;

    if      (IsKeyDown(KEY_W)) gLeftMovement =  1.0f;
    else if (IsKeyDown(KEY_S)) gLeftMovement = -1.0f;

    if (IsKeyPressed(KEY_T)) gSinglePlayer = !gSinglePlayer;

    // in 1-player mode the arrow keys do nothing
    if (gSinglePlayer)
    {
        gRightMovement = gRightDirection;
    }
    else
    {
        if      (IsKeyDown(KEY_UP))   gRightMovement =  1.0f;
        else if (IsKeyDown(KEY_DOWN)) gRightMovement = -1.0f;
    }

    int newBallCount = gBallCount;
    if      (IsKeyPressed(KEY_ONE))   newBallCount = 1;
    else if (IsKeyPressed(KEY_TWO))   newBallCount = 2;
    else if (IsKeyPressed(KEY_THREE)) newBallCount = 3;
    else if (IsKeyPressed(KEY_FOUR))  newBallCount = 4;
    else if (IsKeyPressed(KEY_FIVE))  newBallCount = 5;

    // new balls start back in the middle
    for (int i = gBallCount; i < newBallCount; i++)
    {
        gBallPositions[i] = BALL_START_POS[i];

        for (int j = 0; j < TRAIL_LENGTH; j++)
        {
            gTrailPositions[i][j] = BALL_START_POS[i];
        }
    }
    gBallCount = newBallCount;

    // restart once someone has won (keeps the same number of balls and player mode)
    if (gWinner != 0 && IsKeyPressed(KEY_R)) resetGame();

    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
}

void update()
{
    // Delta time
    float ticks = static_cast<float>(GetTime());
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    if (gWinner != 0) return;

    // paddles slide around the circle
    gLeftAngle  += PADDLE_SPEED * gLeftMovement  * deltaTime;
    gRightAngle += PADDLE_SPEED * gRightMovement * deltaTime;

    if (gLeftAngle >  PADDLE_MAX_ANGLE) gLeftAngle =  PADDLE_MAX_ANGLE;
    if (gLeftAngle < -PADDLE_MAX_ANGLE) gLeftAngle = -PADDLE_MAX_ANGLE;

    if (gRightAngle > PADDLE_MAX_ANGLE)
    {
        gRightAngle     = PADDLE_MAX_ANGLE;
        gRightDirection = -1.0f;
    }
    if (gRightAngle < -PADDLE_MAX_ANGLE)
    {
        gRightAngle     = -PADDLE_MAX_ANGLE;
        gRightDirection = 1.0f;
    }

    // direction from the middle of the circle to each paddle (length 1)
    Vector2 leftDirection  = { -cos(gLeftAngle * DEG_TO_RAD),  -sin(gLeftAngle * DEG_TO_RAD) };
    Vector2 rightDirection = {  cos(gRightAngle * DEG_TO_RAD), -sin(gRightAngle * DEG_TO_RAD) };

    gLeftPosition = {
        ORIGIN.x + PADDLE_RADIUS * leftDirection.x,
        ORIGIN.y + PADDLE_RADIUS * leftDirection.y
    };
    gRightPosition = {
        ORIGIN.x + PADDLE_RADIUS * rightDirection.x,
        ORIGIN.y + PADDLE_RADIUS * rightDirection.y
    };

    // balls
    for (int i = 0; i < gBallCount; i++)
    {
        gBallPositions[i] = {
            gBallPositions[i].x + BALL_SPEED * gBallMovements[i].x * deltaTime,
            gBallPositions[i].y + BALL_SPEED * gBallMovements[i].y * deltaTime
        };

        Vector2 ballDirection = { gBallPositions[i].x - ORIGIN.x, gBallPositions[i].y - ORIGIN.y };
        float   distance      = GetLength(&ballDirection);
        Normalise(&ballDirection);

        // ball is out at the paddles' distance from the middle
        if (distance > PADDLE_RADIUS - PADDLE_THICKNESS / 2.0f - BALL_RADIUS &&
            distance < PADDLE_RADIUS + PADDLE_THICKNESS / 2.0f)
        {
            // how far the ball is from the middle of each paddle
            Vector2 ballToLeft  = { gBallPositions[i].x - gLeftPosition.x,  gBallPositions[i].y - gLeftPosition.y  };
            Vector2 ballToRight = { gBallPositions[i].x - gRightPosition.x, gBallPositions[i].y - gRightPosition.y };

            // paddles bounce like a flat mirror facing the middle, so hitting
            // near the end of a paddle sends the ball off at an angle
            if (GetLength(&ballToLeft) < PADDLE_HIT_RANGE)
            {
                bounceOff(&gBallMovements[i], &leftDirection);
            }
            if (GetLength(&ballToRight) < PADDLE_HIT_RANGE)
            {
                bounceOff(&gBallMovements[i], &rightDirection);
            }
        }

        // ball reached the edge of the circle
        if (distance > ARENA_RADIUS - BALL_RADIUS)
        {
            // top and bottom walls bounce, the rest of the edge is a goal
            float wallWidth = sin(WALL_HALF_ANGLE * DEG_TO_RAD);

            if (fabs(ballDirection.x) < wallWidth)
            {
                bounceOff(&gBallMovements[i], &ballDirection);
            }
            else if (ballDirection.x < 0.0f)
            {
                gWinner = 2;  // went past the left paddle
            }
            else
            {
                gWinner = 1;  // went past the right paddle
            }
        }
    }

    // trail: every TRAIL_INTERVAL seconds, push every point back one and add the new position
    gTrailTimer += deltaTime;
    if (gTrailTimer >= TRAIL_INTERVAL)
    {
        gTrailTimer = 0.0f;

        for (int i = 0; i < gBallCount; i++)
        {
            for (int j = TRAIL_LENGTH - 1; j > 0; j--)
            {
                gTrailPositions[i][j] = gTrailPositions[i][j - 1];
            }
            gTrailPositions[i][0] = gBallPositions[i];
        }
    }
}

void render()
{
    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOUR));

    renderObject(&gBackgroundTexture, &gBackgroundPosition, &gBackgroundScale, 0.0f, WHITE);

    // the paddle texture curves the same way as the left side of the circle,
    // so the right one gets turned around 180 degrees
    renderObject(&gPaddleTexture, &gLeftPosition,  &gPaddleScale, gLeftAngle,           WHITE);
    renderObject(&gPaddleTexture, &gRightPosition, &gPaddleScale, 180.0f - gRightAngle, WHITE);

    for (int i = 0; i < gBallCount; i++)
    {
        // older trail points fade out more
        for (int j = 0; j < TRAIL_LENGTH; j++)
        {
            float fade  = static_cast<float>(TRAIL_LENGTH - j) / (TRAIL_LENGTH + 1);
            float alpha = TRAIL_ALPHA * fade;

            Color trailTint = { 255, 255, 255, static_cast<unsigned char>(alpha) };
            renderObject(&gBallTexture, &gTrailPositions[i][j], &gBallScale, 0.0f, trailTint);
        }

        renderObject(&gBallTexture, &gBallPositions[i], &gBallScale, 0.0f, WHITE);
    }

    // in 1-player mode you're the left paddle playing against the computer
    if (gSinglePlayer)
    {
        if (gWinner == 1) renderObject(&gYouWinTexture,  &gWinPosition, &gWinScale, 0.0f, WHITE);
        if (gWinner == 2) renderObject(&gYouLoseTexture, &gWinPosition, &gWinScale, 0.0f, WHITE);
    }
    else
    {
        if (gWinner == 1) renderObject(&gLeftWinsTexture,  &gWinPosition, &gWinScale, 0.0f, WHITE);
        if (gWinner == 2) renderObject(&gRightWinsTexture, &gWinPosition, &gWinScale, 0.0f, WHITE);
    }

    EndDrawing();
}

void shutdown()
{
    UnloadTexture(gBackgroundTexture);
    UnloadTexture(gPaddleTexture);
    UnloadTexture(gBallTexture);
    UnloadTexture(gLeftWinsTexture);
    UnloadTexture(gRightWinsTexture);
    UnloadTexture(gYouWinTexture);
    UnloadTexture(gYouLoseTexture);
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}
