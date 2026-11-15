/**
* Author: Deyon Wijewardana
* Assignment: Rise of the AI
* Date due: 11/16/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/Menu.h"
#include "CS3113/LevelA.h"
#include "CS3113/LevelB.h"
#include "CS3113/LevelC.h"
#include "CS3113/EndScreen.h"

// Global Constants
constexpr int SCREEN_WIDTH  = 1000,
              SCREEN_HEIGHT = 600,
              FPS           = 120,
              MAX_LIVES     = 3;

constexpr Vector2 ORIGIN = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };

constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

// Global Variables
AppStatus gAppStatus       = RUNNING;
float     gPreviousTicks   = 0.0f,
          gTimeAccumulator = 0.0f;

Camera2D gCamera = {};

Scene                     *gCurrentScene   = nullptr;
SceneID                    gCurrentSceneID = NO_SCENE;
std::map<SceneID, Scene *> gLevels         = {};

int gLives = MAX_LIVES;

// sounds are global so they don't get cut off when a level unloads
Sound gJumpSound       = {},
      gHitSound        = {},
      gLevelClearSound = {};

Texture2D gHeartTexture = {};

// Function Declarations
void switchToScene(SceneID id);
void initialise();
void processInput();
void update();
void render();
void shutdown();

void switchToScene(SceneID id)
{
    if (gCurrentScene) gCurrentScene->shutdown();

    // going back to the menu starts a new game
    if (id == MENU) gLives = MAX_LIVES;

    gCurrentSceneID = id;
    gCurrentScene   = gLevels[id];
    gCurrentScene->initialise();

    // menu and end screens have no player
    if (gCurrentScene->getState().player)
        gCamera.target = gCurrentScene->getState().player->getPosition();
    else
        gCamera.target = ORIGIN;
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Rise of the AI");
    InitAudioDevice();

    gJumpSound       = LoadSound("assets/jump.ogg");
    gHitSound        = LoadSound("assets/hit.ogg");
    gLevelClearSound = LoadSound("assets/level_clear.ogg");

    gHeartTexture = LoadTexture("assets/heart.png");

    gLevels[MENU]    = new Menu(ORIGIN, "#1B1B2F");
    gLevels[LEVEL_A] = new LevelA(ORIGIN, "#8FD3FF");
    gLevels[LEVEL_B] = new LevelB(ORIGIN, "#FFD8A8");
    gLevels[LEVEL_C] = new LevelC(ORIGIN, "#3B4A6B");
    gLevels[WIN]     = new EndScreen(ORIGIN, "#1D3557", "assets/you_win.png");
    gLevels[LOSE]    = new EndScreen(ORIGIN, "#3D0000", "assets/you_lose.png");

    gCamera.offset   = ORIGIN;
    gCamera.rotation = 0.0f;
    gCamera.zoom     = 1.0f;

    switchToScene(MENU);

    SetTargetFPS(FPS);
}

void processInput()
{
    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;

    // debug: jump straight to a level
    if      (IsKeyPressed(KEY_ONE))   switchToScene(LEVEL_A);
    else if (IsKeyPressed(KEY_TWO))   switchToScene(LEVEL_B);
    else if (IsKeyPressed(KEY_THREE)) switchToScene(LEVEL_C);

    if (IsKeyPressed(KEY_ENTER))
    {
        if (gCurrentSceneID == MENU)
            switchToScene(LEVEL_A);
        else if (gCurrentSceneID == WIN || gCurrentSceneID == LOSE)
            switchToScene(MENU);
    }

    Entity *player = gCurrentScene->getState().player;
    if (player == nullptr) return;

    player->resetMovement();

    if      (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  player->moveLeft();
    else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) player->moveRight();

    if ((IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_SPACE)) &&
        player->isCollidingBottom())
    {
        player->jump();
        PlaySound(gJumpSound);
    }

    // to avoid faster diagonal speed
    if (GetLength(player->getMovement()) > 1.0f) player->normaliseMovement();
}

void update()
{
    // Delta time
    float ticks     = static_cast<float>(GetTime());
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    // Fixed timestep
    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP)
    {
        gCurrentScene->update(FIXED_TIMESTEP);
        deltaTime -= FIXED_TIMESTEP;

        Entity *player = gCurrentScene->getState().player;
        Map    *map    = gCurrentScene->getState().map;

        if (player && map)
        {
            Vector2 cameraTarget = { player->getPosition().x, ORIGIN.y };

            // keep the camera from showing past the ends of the map
            float leftLimit  = map->getLeftBoundary()  + SCREEN_WIDTH / 2.0f;
            float rightLimit = map->getRightBoundary() - SCREEN_WIDTH / 2.0f;

            if (cameraTarget.x < leftLimit)  cameraTarget.x = leftLimit;
            if (cameraTarget.x > rightLimit) cameraTarget.x = rightLimit;

            panCamera(&gCamera, &cameraTarget);
        }
    }

    gTimeAccumulator = deltaTime;
}

void render()
{
    BeginDrawing();
    BeginMode2D(gCamera);

    gCurrentScene->render();

    EndMode2D();

    // hearts for lives (after EndMode2D so they don't move with the camera)
    if (gCurrentScene->getState().player)
    {
        for (int i = 0; i < gLives; i++)
        {
            Rectangle textureArea = {
                0.0f, 0.0f,
                static_cast<float>(gHeartTexture.width),
                static_cast<float>(gHeartTexture.height)
            };
            Rectangle destinationArea = { 20.0f + i * 50.0f, 20.0f, 45.0f, 45.0f };

            DrawTexturePro(gHeartTexture, textureArea, destinationArea, { 0.0f, 0.0f }, 0.0f, WHITE);
        }
    }

    EndDrawing();
}

void shutdown()
{
    for (std::pair<const SceneID, Scene *> &entry : gLevels)
        delete entry.second;

    gLevels.clear();

    UnloadSound(gJumpSound);
    UnloadSound(gHitSound);
    UnloadSound(gLevelClearSound);
    UnloadTexture(gHeartTexture);

    CloseAudioDevice();
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();

        if (gCurrentScene->getState().playerDied)
        {
            PlaySound(gHitSound);
            gLives--;

            if (gLives <= 0) switchToScene(LOSE);
            else             switchToScene(gCurrentSceneID); // restart level
        }
        else if (gCurrentScene->getState().nextSceneID != NO_SCENE)
        {
            PlaySound(gLevelClearSound);
            switchToScene(gCurrentScene->getState().nextSceneID);
        }

        render();
    }

    shutdown();

    return 0;
}
