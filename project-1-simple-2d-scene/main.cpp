/**
* Author: Deyon Wijewardana 
* Assignment: Simple 2D Scene
* Date due: 10/05/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/cs3113.h"
#include <math.h>

// Global Constants
constexpr int SCREEN_WIDTH  = 800 * 1.5f,
              SCREEN_HEIGHT = 450 * 1.5f,
              FPS           = 60;

constexpr Vector2 ORIGIN = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };

// byakuya moves in a figure 8 around the middle of the screen
constexpr float BYAKUYA_HEIGHT     = 300.0f,
                BYAKUYA_SPEED      = 0.4f,  // how fast he goes around the 8
                FIGURE_8_WIDTH     = 200.0f,
                FIGURE_8_HEIGHT    = 70.0f,
                BYAKUYA_FLASH_TIME = 2.5f,  // seconds between flash steps
                BYAKUYA_FLASH_JUMP = 1.2f;  // how far ahead on the 8 he jumps

// ichigo orbits around byakuya (wherever byakuya is)
constexpr float ICHIGO_HEIGHT     = 280.0f,
                ICHIGO_SPEED      = 0.8f,
                ICHIGO_RADIUS_X   = 300.0f,
                ICHIGO_RADIUS_Y   = 100.0f,
                ICHIGO_FLASH_TIME = 1.5f,
                ICHIGO_FLASH_JUMP = 2.09f; // about a third of the circle 

// senbonzakura petals orbit byakuya the other way and spin
constexpr float PETAL_SIZE   = 220.0f,
                PETAL_RADIUS = 170.0f,
                PETAL_SPEED  = -1.5f,  // negative = counter clockwise
                PETAL_SPIN   = 200.0f; // degrees per second

// getsuga tensho flies across the screen in a wave and pulses
constexpr float GETSUGA_SIZE        = 160.0f,
                GETSUGA_SPEED       = 250.0f, // pixels per second
                GETSUGA_WAVE_HEIGHT = 180.0f,
                GETSUGA_WAVE_SPEED  = 2.0f,
                GETSUGA_PULSE_SIZE  = 30.0f,
                GETSUGA_PULSE_SPEED = 6.0f;

// background fades back and forth between these two
constexpr char  BG_COLOUR_A[] = "#B9C6D6",
                BG_COLOUR_B[] = "#C76B86";
constexpr float BG_SPEED      = 0.5f;

constexpr char ICHIGO_FP[]  = "assets/ichigo.png",
               BYAKUYA_FP[] = "assets/byakuya.png",
               PETAL_FP[]   = "assets/senbonzakura.png",
               GETSUGA_FP[] = "assets/getsuga_tensho.png";

// Global Variables
AppStatus gAppStatus     = RUNNING;
float     gPreviousTicks = 0.0f;

Texture2D gIchigoTexture,
          gByakuyaTexture,
          gPetalTexture,
          gGetsugaTexture;

// byakuya
Vector2 gByakuyaPosition   = ORIGIN,
        gByakuyaScale;
float   gByakuyaPathTime   = 0.0f,
        gByakuyaFlashTimer = 0.0f;

// ichigo
Vector2 gIchigoPosition   = ORIGIN,
        gIchigoScale;
float   gIchigoAngle      = 0.0f,
        gIchigoFlashTimer = 0.0f;

// petals
Vector2 gPetalPosition   = ORIGIN;
float   gPetalOrbitAngle = 0.0f,
        gPetalSpinAngle  = 0.0f;

// getsuga tensho
Vector2 gGetsugaPosition = { -GETSUGA_SIZE, ORIGIN.y },
        gGetsugaScale    = { GETSUGA_SIZE, GETSUGA_SIZE };
float   gGetsugaTime     = 0.0f;

// background
float gBackgroundTime   = 0.0f;
Color gBackgroundColour = ColorFromHex(BG_COLOUR_A);

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

// Function Definitions
void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Ichigo vs Byakuya");

    gIchigoTexture  = LoadTexture(ICHIGO_FP);
    gByakuyaTexture = LoadTexture(BYAKUYA_FP);
    gPetalTexture   = LoadTexture(PETAL_FP);
    gGetsugaTexture = LoadTexture(GETSUGA_FP);

    // the character pictures aren't square, so keeping their width/height ratio
    gIchigoScale = {
        ICHIGO_HEIGHT * gIchigoTexture.width / gIchigoTexture.height,
        ICHIGO_HEIGHT
    };
    gByakuyaScale = {
        BYAKUYA_HEIGHT * gByakuyaTexture.width / gByakuyaTexture.height,
        BYAKUYA_HEIGHT
    };

    SetTargetFPS(FPS);
}

void processInput()
{
    if (WindowShouldClose()) gAppStatus = TERMINATED;
}

void update()
{
    // Delta time
    float ticks = static_cast<float>(GetTime());
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;

    /**
     * BYAKUYA: figure 8 + flash step
     */
    gByakuyaPathTime   += BYAKUYA_SPEED * deltaTime;
    gByakuyaFlashTimer += deltaTime;

    // every few seconds he flash steps (skips ahead on the 8)
    if (gByakuyaFlashTimer >= BYAKUYA_FLASH_TIME)
    {
        gByakuyaFlashTimer = 0.0f;
        gByakuyaPathTime  += BYAKUYA_FLASH_JUMP;
    }

    // using sin(t) for x and sin(2t) to get the figure 8 shape
    gByakuyaPosition = {
        ORIGIN.x + FIGURE_8_WIDTH  * sin(gByakuyaPathTime),
        ORIGIN.y + FIGURE_8_HEIGHT * sin(2.0f * gByakuyaPathTime)
    };

    /**
     * ICHIGO: orbits byakuya + flash step
     */
    gIchigoAngle      += ICHIGO_SPEED * deltaTime;
    gIchigoFlashTimer += deltaTime;

    if (gIchigoFlashTimer >= ICHIGO_FLASH_TIME)
    {
        gIchigoFlashTimer = 0.0f;
        gIchigoAngle     += ICHIGO_FLASH_JUMP;
    }

    // orbit is relative to byakuya's position, not the middle of the screen
    gIchigoPosition = {
        gByakuyaPosition.x + ICHIGO_RADIUS_X * cos(gIchigoAngle),
        gByakuyaPosition.y + ICHIGO_RADIUS_Y * sin(gIchigoAngle)
    };

    /**
     * SENBONZAKURA: petals orbit byakuya and spin
     */
    gPetalOrbitAngle += PETAL_SPEED * deltaTime;
    gPetalSpinAngle  += PETAL_SPIN  * deltaTime;

    gPetalPosition = {
        gByakuyaPosition.x + PETAL_RADIUS * cos(gPetalOrbitAngle),
        gByakuyaPosition.y + PETAL_RADIUS * sin(gPetalOrbitAngle)
    };

    /**
     * GETSUGA TENSHO: wave across the screen + pulse
     */
    gGetsugaTime += deltaTime;

    gGetsugaPosition.x += GETSUGA_SPEED * deltaTime;
    gGetsugaPosition.y  = ORIGIN.y + GETSUGA_WAVE_HEIGHT * sin(GETSUGA_WAVE_SPEED * gGetsugaTime);

    // once it goes off the right side, start again from the left
    if (gGetsugaPosition.x > SCREEN_WIDTH + GETSUGA_SIZE) gGetsugaPosition.x = -GETSUGA_SIZE;

    gGetsugaScale = {
        GETSUGA_SIZE + GETSUGA_PULSE_SIZE * cos(GETSUGA_PULSE_SPEED * gGetsugaTime),
        GETSUGA_SIZE + GETSUGA_PULSE_SIZE * cos(GETSUGA_PULSE_SPEED * gGetsugaTime)
    };

    /**
     * BACKGROUND: fade between the two colours
     */
    gBackgroundTime += BG_SPEED * deltaTime;

    // sin goes from -1 to 1, this turns it into 0 to 1
    float mix = (sin(gBackgroundTime) + 1.0f) / 2.0f;

    Color colourA = ColorFromHex(BG_COLOUR_A);
    Color colourB = ColorFromHex(BG_COLOUR_B);

    gBackgroundColour = {
        static_cast<unsigned char>(colourA.r + (colourB.r - colourA.r) * mix),
        static_cast<unsigned char>(colourA.g + (colourB.g - colourA.g) * mix),
        static_cast<unsigned char>(colourA.b + (colourB.b - colourA.b) * mix),
        255
    };
}

void render()
{
    BeginDrawing();

    ClearBackground(gBackgroundColour);

    /**
     * BYAKUYA
     */
    Rectangle byakuyaTextureArea = {
        0.0f, 0.0f,
        static_cast<float>(gByakuyaTexture.width),
        static_cast<float>(gByakuyaTexture.height)
    };

    Rectangle byakuyaDestinationArea = {
        gByakuyaPosition.x,
        gByakuyaPosition.y,
        gByakuyaScale.x,
        gByakuyaScale.y
    };

    Vector2 byakuyaOrigin = {
        gByakuyaScale.x / 2.0f,
        gByakuyaScale.y / 2.0f
    };

    DrawTexturePro(
        gByakuyaTexture,
        byakuyaTextureArea,
        byakuyaDestinationArea,
        byakuyaOrigin,
        0.0f,
        WHITE
    );

    /**
     * SENBONZAKURA
     */
    Rectangle petalTextureArea = {
        0.0f, 0.0f,
        static_cast<float>(gPetalTexture.width),
        static_cast<float>(gPetalTexture.height)
    };

    Rectangle petalDestinationArea = {
        gPetalPosition.x,
        gPetalPosition.y,
        PETAL_SIZE,
        PETAL_SIZE
    };

    Vector2 petalOrigin = {
        PETAL_SIZE / 2.0f,
        PETAL_SIZE / 2.0f
    };

    DrawTexturePro(
        gPetalTexture,
        petalTextureArea,
        petalDestinationArea,
        petalOrigin,
        gPetalSpinAngle,
        WHITE
    );

    /**
     * ICHIGO
     */
    Rectangle ichigoTextureArea = {
        0.0f, 0.0f,
        static_cast<float>(gIchigoTexture.width),
        static_cast<float>(gIchigoTexture.height)
    };

    Rectangle ichigoDestinationArea = {
        gIchigoPosition.x,
        gIchigoPosition.y,
        gIchigoScale.x,
        gIchigoScale.y
    };

    Vector2 ichigoOrigin = {
        gIchigoScale.x / 2.0f,
        gIchigoScale.y / 2.0f
    };

    DrawTexturePro(
        gIchigoTexture,
        ichigoTextureArea,
        ichigoDestinationArea,
        ichigoOrigin,
        0.0f,
        WHITE
    );

    /**
     * GETSUGA TENSHO
     */
    Rectangle getsugaTextureArea = {
        0.0f, 0.0f,
        static_cast<float>(gGetsugaTexture.width),
        static_cast<float>(gGetsugaTexture.height)
    };

    Rectangle getsugaDestinationArea = {
        gGetsugaPosition.x,
        gGetsugaPosition.y,
        gGetsugaScale.x,
        gGetsugaScale.y
    };

    Vector2 getsugaOrigin = {
        gGetsugaScale.x / 2.0f,
        gGetsugaScale.y / 2.0f
    };

    DrawTexturePro(
        gGetsugaTexture,
        getsugaTextureArea,
        getsugaDestinationArea,
        getsugaOrigin,
        0.0f,
        WHITE
    );

    EndDrawing();
}

void shutdown()
{
    UnloadTexture(gIchigoTexture);
    UnloadTexture(gByakuyaTexture);
    UnloadTexture(gPetalTexture);
    UnloadTexture(gGetsugaTexture);

    CloseWindow(); // Close window and OpenGL context
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
