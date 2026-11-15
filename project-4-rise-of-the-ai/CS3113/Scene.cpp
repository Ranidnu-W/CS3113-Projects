/**
* Author: Deyon Wijewardana
* Assignment: Rise of the AI
* Date due: 11/16/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "Scene.h"

Scene::Scene() : mGameState{}, mOrigin{} {}

Scene::Scene(Vector2 origin, const char *bgHexCode)
    : mGameState{}, mOrigin{ origin }, mBGColourHexCode{ bgHexCode }
{}

// centre of a map tile in world coordinates
Vector2 Scene::tileToWorld(int col, int row)
{
    float tileSize = mGameState.map->getTileSize();

    return {
        mGameState.map->getLeftBoundary() + col * tileSize + tileSize / 2.0f,
        mGameState.map->getTopBoundary()  + row * tileSize + tileSize / 2.0f
    };
}

void Scene::addEnemy(AIType type, int col, int row, int speed)
{
    if (mGameState.enemyCount >= MAX_ENEMIES) return;

    // frames in characters.png (3 rows x 9 cols)
    std::vector<int> frames;

    if (type == WANDERER)      frames = { 15, 16 };
    else if (type == FOLLOWER) frames = { 21, 22 };
    else                       frames = { 24, 25, 26, 25 };

    // the robots look the same in every direction
    std::map<Direction, std::vector<int>> atlas = {
        { LEFT,  frames },
        { RIGHT, frames },
        { UP,    frames },
        { DOWN,  frames },
    };

    Entity *enemy = new Entity(
        tileToWorld(col, row),    // position
        { 78.0f, 78.0f },         // scale
        "assets/characters.png",  // texture file address
        ATLAS,                    // single image or atlas?
        { 3, 9 },                 // atlas dimensions
        atlas,                    // actual atlas
        NPC                       // entity type
    );

    enemy->setAIType(type);
    enemy->setAIState(IDLE);
    enemy->setSpeed(speed);
    enemy->setFrameSpeed(6);

    if (type == WANDERER)
    {
        enemy->setColliderDimensions({ 44.0f, 72.0f });
        enemy->setAcceleration({ 0.0f, 981.0f });
        enemy->setDirection(LEFT);
    }
    else if (type == FOLLOWER)
    {
        enemy->setColliderDimensions({ 60.0f, 72.0f });
        enemy->setAcceleration({ 0.0f, 981.0f });
        enemy->setDirection(LEFT);
    }
    else // FLYER
    {
        enemy->setColliderDimensions({ 48.0f, 40.0f });
        enemy->setAcceleration({ 0.0f, 0.0f });
        enemy->moveDown();
    }

    mGameState.enemies[mGameState.enemyCount] = enemy;
    mGameState.enemyCount++;
}

void Scene::drawCentred(Texture2D texture, Vector2 position)
{
    Rectangle textureArea = {
        0.0f, 0.0f,
        static_cast<float>(texture.width),
        static_cast<float>(texture.height)
    };

    Rectangle destinationArea = {
        position.x, position.y,
        static_cast<float>(texture.width),
        static_cast<float>(texture.height)
    };

    Vector2 originOffset = { texture.width / 2.0f, texture.height / 2.0f };

    DrawTexturePro(texture, textureArea, destinationArea, originOffset, 0.0f, WHITE);
}
