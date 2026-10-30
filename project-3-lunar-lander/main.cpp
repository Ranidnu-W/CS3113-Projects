/**
* Author: Deyon Wijewardana
* Assignment: Lunar Lander!
* Date due: 10/26/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/Entity.h"

// Lunar Lander: land the ship softly on one of the two landing pads. One pad
// sits in a valley on the right, the other one floats and slides back and
// forth. Touching the rocks, hitting a pad too fast, or flying off the screen
// fails the mission. The engines burn fuel, and with no fuel they stop working.

enum GameStatus { PLAYING, WON, LOST };

// Global Constants
constexpr int SCREEN_WIDTH  = 1200,
              SCREEN_HEIGHT = 675,
              FPS           = 120;

constexpr char    BG_COLOUR[] = "#04050E";
constexpr Vector2 ORIGIN      = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };

// accelerations are in pixels/s²
constexpr float FIXED_TIMESTEP          = 1.0f / 60.0f,
                ACCELERATION_OF_GRAVITY = 50.0f,
                MAIN_ENGINE_THRUST      = 110.0f,
                SIDE_ENGINE_THRUST      = 90.0f,
                DRAG                    = 1.2f,
                MAX_FUEL                = 100.0f,
                MAIN_ENGINE_FUEL_USE    = 6.0f,  // fuel per second
                SIDE_ENGINE_FUEL_USE    = 4.0f;

constexpr Vector2 SHIP_START    = { 150.0f, 150.0f },
                  SHIP_SIZE     = { 48.0f, 72.0f },
                  SHIP_COLLIDER = { 30.0f, 36.0f }; // just the body, not the flames

// how many rock tiles are stacked in each column, from left to right
constexpr int   NUMBER_OF_COLUMNS = 24;
constexpr int   TERRAIN_HEIGHTS[NUMBER_OF_COLUMNS] = {
    4, 4, 3, 3, 2, 2, 1, 1, 2, 2, 1, 1,
    2, 3, 5, 6, 5, 4, 3, 2, 2, 2, 3, 4
};
constexpr float TILE_DIMENSION = 50.0f;

constexpr Vector2 LANDING_PAD_POSITION = { 1025.0f, 567.0f },
                  LANDING_PAD_SIZE     = { 140.0f, 16.0f },
                  MOVING_PAD_START     = { 600.0f, 330.0f },
                  MOVING_PAD_SIZE      = { 110.0f, 16.0f };
constexpr float   MOVING_PAD_LEFT      = 420.0f,
                  MOVING_PAD_RIGHT     = 780.0f;
constexpr int     MOVING_PAD_SPEED     = 100;

// HUD
constexpr float FUEL_BAR_LEFT   = 113.0f,
                FUEL_BAR_WIDTH  = 214.0f,
                FUEL_BAR_HEIGHT = 16.0f,
                FUEL_BAR_Y      = 30.0f;

// Global Variables
AppStatus  gAppStatus       = RUNNING;
GameStatus gGameStatus      = PLAYING;
float      gPreviousTicks   = 0.0f,
           gTimeAccumulator = 0.0f,
           gFuel            = MAX_FUEL;
bool       gMainEngineOn    = false,
           gSideEngineOn    = false;
int        gNumberOfMapTiles = 0;

Entity *gShip       = nullptr;
Entity *gMap        = nullptr; // rocks + both landing pads
Entity *gMovingPad  = nullptr;
Entity *gBackground = nullptr;
Entity *gExplosion  = nullptr;

Entity *gMissionAccomplished = nullptr;
Entity *gMissionFailed       = nullptr;
Entity *gFuelLabel           = nullptr;
Entity *gFuelBarFrame        = nullptr;
Entity *gFuelBarFill         = nullptr;
Entity *gSpeedLabel          = nullptr;
Entity *gSafeLight           = nullptr;
Entity *gDangerLight         = nullptr;
Entity *gControls            = nullptr;

// Function Declarations
void initialise();
void resetGame();
void processInput();
void update();
void render();
void shutdown();

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lunar Lander!");

    gBackground = new Entity(
        ORIGIN, { SCREEN_WIDTH, SCREEN_HEIGHT }, "assets/background.png", NONE
    );

    /*
        ----------- SHIP -----------
    */
    std::map<Direction, std::vector<int>> shipAtlas = {
        { DOWN,  {  0,  1,  2,  3 } }, // engines off
        { UP,    {  4,  5,  6,  7 } }, // main engine
        { LEFT,  {  8,  9, 10, 11 } }, // steering left
        { RIGHT, { 12, 13, 14, 15 } }, // steering right
    };

    gShip = new Entity(
        SHIP_START,          // position
        SHIP_SIZE,           // scale
        "assets/ship.png",   // texture file address
        ATLAS,               // single image or atlas?
        { 4, 4 },            // atlas dimensions
        shipAtlas,           // actual atlas
        PLAYER               // entity type
    );
    gShip->setColliderDimensions(SHIP_COLLIDER);
    gShip->setFrameSpeed(10);

    gExplosion = new Entity(
        ORIGIN, { 90.0f, 90.0f }, "assets/explosion.png", NONE
    );

    /*
        ----------- MAP (rocks + landing pads) -----------
    */
    int numberOfRocks = 0;
    for (int i = 0; i < NUMBER_OF_COLUMNS; i++)
        numberOfRocks += TERRAIN_HEIGHTS[i];

    gNumberOfMapTiles = numberOfRocks + 2;
    gMap = new Entity[gNumberOfMapTiles];

    int tile = 0;
    for (int column = 0; column < NUMBER_OF_COLUMNS; column++)
    {
        for (int row = 0; row < TERRAIN_HEIGHTS[column]; row++)
        {
            // the top tile of each column gets the dusty surface texture
            if (row == TERRAIN_HEIGHTS[column] - 1)
                gMap[tile].setTexture("assets/rock_top.png");
            else
                gMap[tile].setTexture("assets/rock.png");

            gMap[tile].setEntityType(BLOCK);
            gMap[tile].setScale({ TILE_DIMENSION, TILE_DIMENSION });
            gMap[tile].setColliderDimensions({ TILE_DIMENSION, TILE_DIMENSION });
            gMap[tile].setPosition({
                TILE_DIMENSION / 2.0f + column * TILE_DIMENSION,
                SCREEN_HEIGHT - TILE_DIMENSION / 2.0f - row * TILE_DIMENSION
            });

            tile++;
        }
    }

    // landing pad in the valley on the right
    Entity *landingPad = &gMap[numberOfRocks];
    landingPad->setTexture("assets/landing_pad.png");
    landingPad->setEntityType(PLATFORM);
    landingPad->setScale(LANDING_PAD_SIZE);
    landingPad->setColliderDimensions(LANDING_PAD_SIZE);
    landingPad->setPosition(LANDING_PAD_POSITION);

    // landing pad that slides left and right
    gMovingPad = &gMap[numberOfRocks + 1];
    gMovingPad->setTexture("assets/moving_pad.png");
    gMovingPad->setEntityType(PLATFORM);
    gMovingPad->setScale(MOVING_PAD_SIZE);
    gMovingPad->setColliderDimensions(MOVING_PAD_SIZE);
    gMovingPad->setSpeed(MOVING_PAD_SPEED);

    /*
        ----------- HUD -----------
    */
    gFuelLabel    = new Entity({ 62.0f, FUEL_BAR_Y }, { 64.0f, 20.0f }, "assets/fuel_label.png", NONE);
    gFuelBarFrame = new Entity({ 220.0f, FUEL_BAR_Y }, { 220.0f, 22.0f }, "assets/fuel_bar_frame.png", NONE);
    gFuelBarFill  = new Entity({ 220.0f, FUEL_BAR_Y }, { FUEL_BAR_WIDTH, FUEL_BAR_HEIGHT }, "assets/fuel_bar_fill.png", NONE);
    gSpeedLabel   = new Entity({ 62.0f, 60.0f }, { 64.0f, 20.0f }, "assets/speed_label.png", NONE);
    gSafeLight    = new Entity({ 128.0f, 60.0f }, { 24.0f, 24.0f }, "assets/light_safe.png", NONE);
    gDangerLight  = new Entity({ 128.0f, 60.0f }, { 24.0f, 24.0f }, "assets/light_danger.png", NONE);
    gControls     = new Entity({ 990.0f, 36.0f }, { 380.0f, 52.0f }, "assets/controls.png", NONE);

    // up in the sky so the banner doesn't cover the pads
    gMissionAccomplished = new Entity({ ORIGIN.x, 175.0f }, { 720.0f, 170.0f }, "assets/mission_accomplished.png", NONE);
    gMissionFailed       = new Entity({ ORIGIN.x, 175.0f }, { 720.0f, 170.0f }, "assets/mission_failed.png", NONE);

    resetGame();

    SetTargetFPS(FPS);
}

void resetGame()
{
    gShip->activate();
    gShip->setPosition(SHIP_START);
    gShip->setVelocity({ 0.0f, 0.0f });
    gShip->setAcceleration({ 0.0f, ACCELERATION_OF_GRAVITY });
    gShip->setDirection(DOWN);

    gMovingPad->setPosition(MOVING_PAD_START);
    gMovingPad->moveRight();

    gFuel         = MAX_FUEL;
    gMainEngineOn = false;
    gSideEngineOn = false;

    gFuelBarFill->setScale({ FUEL_BAR_WIDTH, FUEL_BAR_HEIGHT });
    gFuelBarFill->setPosition({ FUEL_BAR_LEFT + FUEL_BAR_WIDTH / 2.0f, FUEL_BAR_Y });

    gTimeAccumulator = 0.0f;
    gGameStatus      = PLAYING;
}

void processInput()
{
    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
    if (IsKeyPressed(KEY_R)) resetGame();

    if (gGameStatus != PLAYING) return;

    // gravity is always pulling the ship down, the engines add on top of it
    Vector2 acceleration = { 0.0f, ACCELERATION_OF_GRAVITY };

    gMainEngineOn = false;
    gSideEngineOn = false;
    gShip->setDirection(DOWN);

    // once the fuel runs out, the engine keys stop working
    if (gFuel > 0.0f)
    {
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
        {
            acceleration.x = -SIDE_ENGINE_THRUST;
            gSideEngineOn  = true;
            gShip->setDirection(LEFT);
        }
        else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
        {
            acceleration.x = SIDE_ENGINE_THRUST;
            gSideEngineOn  = true;
            gShip->setDirection(RIGHT);
        }

        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
        {
            acceleration.y -= MAIN_ENGINE_THRUST;
            gMainEngineOn   = true;
            gShip->setDirection(UP);
        }
    }

    // when the side engine is off, drag pushes against the sideways speed,
    // so the ship drifts for a bit and then comes to a stop
    if (!gSideEngineOn) acceleration.x = -gShip->getVelocity().x * DRAG;

    gShip->setAcceleration(acceleration);
}

void update()
{
    // Delta time
    float ticks     = static_cast<float>(GetTime());
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    // the mission is over, so everything stays frozen
    if (gGameStatus != PLAYING) return;

    // Fixed timestep
    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP)
    {
        gShip->update(FIXED_TIMESTEP, gMap, gNumberOfMapTiles);

        for (int i = 0; i < gNumberOfMapTiles; i++)
            gMap[i].update(FIXED_TIMESTEP, nullptr, 0);

        // the moving pad turns around at the ends of its path
        if (gMovingPad->getPosition().x > MOVING_PAD_RIGHT)
            gMovingPad->moveLeft();
        else if (gMovingPad->getPosition().x < MOVING_PAD_LEFT)
            gMovingPad->moveRight();

        // the engines burn fuel while they're on
        if (gMainEngineOn) gFuel -= MAIN_ENGINE_FUEL_USE * FIXED_TIMESTEP;
        if (gSideEngineOn) gFuel -= SIDE_ENGINE_FUEL_USE * FIXED_TIMESTEP;
        if (gFuel < 0.0f)  gFuel = 0.0f;

        // the fuel bar shrinks but stays lined up on the left
        float fuelWidth = FUEL_BAR_WIDTH * gFuel / MAX_FUEL;
        gFuelBarFill->setScale({ fuelWidth, FUEL_BAR_HEIGHT });
        gFuelBarFill->setPosition({ FUEL_BAR_LEFT + fuelWidth / 2.0f, FUEL_BAR_Y });

        // flying off the screen loses the ship too
        Vector2 shipPosition = gShip->getPosition();
        bool    isOffScreen  = shipPosition.x < 0.0f ||
                               shipPosition.x > SCREEN_WIDTH ||
                               shipPosition.y < 0.0f;

        if (gShip->hasCrashed() || isOffScreen)
        {
            gGameStatus = LOST;
            gExplosion->setPosition(shipPosition);
            gShip->deactivate();
            break;
        }
        else if (gShip->hasLanded())
        {
            gGameStatus = WON;
            gShip->setDirection(DOWN); // engines off
            break;
        }

        deltaTime -= FIXED_TIMESTEP;
    }

    gTimeAccumulator = deltaTime;
}

void render()
{
    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOUR));

    gBackground->render();

    for (int i = 0; i < gNumberOfMapTiles; i++) gMap[i].render();

    gShip->render();
    if (gGameStatus == LOST) gExplosion->render();

    // HUD
    gFuelLabel->render();
    gFuelBarFrame->render();
    gFuelBarFill->render();
    gSpeedLabel->render();
    gControls->render();

    // green = slow enough to land, red = too fast
    if (gShip->getVelocity().y <= Entity::SAFE_LANDING_SPEED) gSafeLight->render();
    else                                                       gDangerLight->render();

    if (gGameStatus == WON)  gMissionAccomplished->render();
    if (gGameStatus == LOST) gMissionFailed->render();

    EndDrawing();
}

void shutdown()
{
    // Unload the textures while the window (GL context) still exists
    delete gShip;
    delete[] gMap;
    delete gBackground;
    delete gExplosion;
    delete gMissionAccomplished;
    delete gMissionFailed;
    delete gFuelLabel;
    delete gFuelBarFrame;
    delete gFuelBarFill;
    delete gSpeedLabel;
    delete gSafeLight;
    delete gDangerLight;
    delete gControls;
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
