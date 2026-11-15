/**
* Author: Deyon Wijewardana
* Assignment: Rise of the AI
* Date due: 11/16/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "LevelD.h"

LevelD::LevelD()
    : Scene{ { 0.0f, 0.0f }, "#000000" }
{}

LevelD::LevelD(Vector2 origin, const char *bgHexCode)
    : Scene{ origin, bgHexCode }
{}

LevelD::~LevelD() { shutdown(); }

void LevelD::initialise()
{
    mGameState.nextSceneID = NO_SCENE;
    mGameState.playerDied  = false;
    mGameState.enemyCount  = 0;

    mBossHealth = BOSS_HEALTH;
    mStunTimer  = 0.0f;

    mGameState.bgm = LoadMusicStream("assets/boss_music.mp3");
    SetMusicVolume(mGameState.bgm, 0.2f);
    PlayMusicStream(mGameState.bgm);

    mStompSound  = LoadSound("assets/stomp.ogg");
    mHintTexture = LoadTexture("assets/boss_hint.png");

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
        ----------- BOSS -----------
    */
    // bigger version of the blue robot
    std::map<Direction, std::vector<int>> bossAnimationAtlas = {
        { RIGHT, { 21, 22 } },
        { LEFT,  { 21, 22 } },
        { UP,    { 21, 22 } },
        { DOWN,  { 21, 22 } },
    };

    Entity *boss = new Entity(
        tileToWorld(16, 7),           // position
        { 117.0f, 117.0f },           // scale
        "assets/characters.png",      // texture file address
        ATLAS,                        // single image or atlas?
        { 3, 9 },                     // atlas dimensions
        bossAnimationAtlas,           // actual atlas
        NPC                           // entity type
    );

    boss->setAIType(FOLLOWER);
    boss->setAIState(WALKING);
    boss->setSpeed(90);
    boss->setFrameSpeed(6);
    boss->setColliderDimensions({ 100.0f, 104.0f });
    boss->setAcceleration({ 0.0f, ACCELERATION_OF_GRAVITY });
    boss->setDirection(LEFT);

    mGameState.enemies[0] = boss;
    mGameState.enemyCount = 1;
}

void LevelD::update(float deltaTime)
{
    UpdateMusicStream(mGameState.bgm);

    Entity *player = mGameState.player;
    Entity *boss   = mGameState.enemies[0];

    player->update(
        deltaTime,      // delta time / fixed timestep
        nullptr,        // player
        mGameState.map, // map
        nullptr,        // collidable entities
        0               // col. entity count
    );

    boss->update(
        deltaTime,      // delta time / fixed timestep
        player,         // player
        mGameState.map, // map
        nullptr,        // collidable entities
        0               // col. entity count
    );

    // the boss is stunned for a bit after each hit
    if (mStunTimer > 0.0f)
    {
        mStunTimer -= deltaTime;

        // it gets faster after each hit
        if (mStunTimer <= 0.0f)
        {
            int hitsTaken = BOSS_HEALTH - mBossHealth;
            boss->setSpeed(90 + hitsTaken * 50);
        }
    }
    else if (player->isColliding(boss))
    {
        float playerBottom = player->getPosition().y + player->getColliderDimensions().y / 2.0f;
        float bossTop      = boss->getPosition().y   - boss->getColliderDimensions().y / 2.0f;

        // jumping on its head hurts the boss, touching it anywhere else hurts the player
        if (player->getVelocity().y > 0.0f && playerBottom < bossTop + 30.0f)
        {
            mBossHealth--;
            PlaySound(mStompSound);

            player->setVelocity({ 0.0f, -500.0f }); // bounce off its head

            if (mBossHealth <= 0)
            {
                boss->deactivate();
                mGameState.nextSceneID = WIN;
            }
            else
            {
                mStunTimer = STUN_TIME;
                boss->setSpeed(0);
            }
        }
        else
        {
            mGameState.playerDied = true;
        }
    }

    if (player->getPosition().y >
        mGameState.map->getBottomBoundary() + FALL_DEATH_DISTANCE)
        mGameState.playerDied = true;
}

void LevelD::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    drawCentred(mHintTexture, tileToWorld(13, 1));

    mGameState.map->render();
    mGameState.enemies[0]->render();
    mGameState.player->render();
}

void LevelD::shutdown()
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
    UnloadSound(mStompSound);
    mStompSound = {};
    UnloadTexture(mHintTexture);
    mHintTexture = {};
}
