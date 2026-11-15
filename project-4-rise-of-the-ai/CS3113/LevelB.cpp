/**
* Author: Deyon Wijewardana
* Assignment: Rise of the AI
* Date due: 11/16/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "LevelB.h"

LevelB::LevelB()
    : Scene{ { 0.0f, 0.0f }, "#000000" }
{}

LevelB::LevelB(Vector2 origin, const char *bgHexCode)
    : Scene{ origin, bgHexCode }
{}

LevelB::~LevelB() { shutdown(); }

void LevelB::initialise()
{
    mGameState.nextSceneID = NO_SCENE;
    mGameState.playerDied  = false;
    mGameState.enemyCount  = 0;

    mGameState.bgm = LoadMusicStream("assets/level_music.mp3");
    SetMusicVolume(mGameState.bgm, 0.2f);
    PlayMusicStream(mGameState.bgm);

    /*
        ----------- MAP -----------
    */
    mGameState.map = new Map(
        LEVEL_WIDTH, LEVEL_HEIGHT, // map grid cols & rows
        mLevelData,                // grid data
        "assets/tileset.png",      // texture filepath
        TILE_DIMENSION,            // tile size
        20, 9,                     // texture cols & rows
        mOrigin                    // in-game origin
    );

    /*
        ----------- PLAYER -----------
    */
    // astronaut frames in characters.png
    std::map<Direction, std::vector<int>> playerAnimationAtlas = {
        { RIGHT, { 0, 1 } },
        { LEFT,  { 0, 1 } },
        { UP,    { 0, 1 } },
        { DOWN,  { 0, 1 } },
    };

    mGameState.player = new Entity(
        tileToWorld(2, 8),            // position
        { 78.0f, 78.0f },             // scale
        "assets/characters.png",      // texture file address
        ATLAS,                        // single image or atlas?
        { 3, 9 },                     // atlas dimensions
        playerAnimationAtlas,         // actual atlas
        PLAYER                        // entity type
    );

    mGameState.player->setSpeed(250);
    mGameState.player->setJumpingPower(600.0f);
    mGameState.player->setFrameSpeed(8);
    mGameState.player->setColliderDimensions({ 44.0f, 72.0f });
    mGameState.player->setAcceleration({ 0.0f, ACCELERATION_OF_GRAVITY });

    /*
        ----------- ENEMIES -----------
    */
    // type, column, row, speed
    addEnemy(WANDERER, 15, 8, 90);
    addEnemy(FLYER,   10, 6, 90);
    addEnemy(FLYER,   22, 5, 90);
    addEnemy(FOLLOWER, 27, 8, 100);
    addEnemy(WANDERER, 36, 8, 90);
}

void LevelB::update(float deltaTime)
{
    UpdateMusicStream(mGameState.bgm);

    mGameState.player->update(
        deltaTime,      // delta time / fixed timestep
        nullptr,        // player
        mGameState.map, // map
        nullptr,        // collidable entities
        0               // col. entity count
    );

    for (int i = 0; i < mGameState.enemyCount; i++)
    {
        mGameState.enemies[i]->update(
            deltaTime,         // delta time / fixed timestep
            mGameState.player, // player
            mGameState.map,    // map
            nullptr,           // collidable entities
            0                  // col. entity count
        );

        // did the player touch this enemy?
        if (mGameState.player->isColliding(mGameState.enemies[i]))
            mGameState.playerDied = true;
    }

    // fell in a pit
    if (mGameState.player->getPosition().y >
        mGameState.map->getBottomBoundary() + FALL_DEATH_DISTANCE)
        mGameState.playerDied = true;

    // reached the right end of the map
    if (mGameState.player->getPosition().x >
        mGameState.map->getRightBoundary() - TILE_DIMENSION * 2.0f)
        mGameState.nextSceneID = LEVEL_C;
}

void LevelB::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    mGameState.map->render();

    for (int i = 0; i < mGameState.enemyCount; i++)
        mGameState.enemies[i]->render();

    mGameState.player->render();
}

void LevelB::shutdown()
{
    delete mGameState.player;
    mGameState.player = nullptr;
    delete mGameState.map;
    mGameState.map = nullptr;

    for (int i = 0; i < mGameState.enemyCount; i++)
        delete mGameState.enemies[i];
    mGameState.enemyCount = 0;

    UnloadMusicStream(mGameState.bgm);
    mGameState.bgm = {};
}
