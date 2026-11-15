#ifndef SCENE_H
#define SCENE_H

#include "Entity.h"

constexpr int MAX_ENEMIES = 10;

struct GameState
{
    Entity *player;
    Map    *map;

    Entity *enemies[MAX_ENEMIES];
    int     enemyCount;

    Music bgm;

    SceneID nextSceneID;
    bool    playerDied;
};

class Scene
{
protected:
    GameState   mGameState;
    Vector2     mOrigin;
    const char *mBGColourHexCode = "#000000";

    void    addEnemy(AIType type, int col, int row, int speed);
    Vector2 tileToWorld(int col, int row);
    void    drawCentred(Texture2D texture, Vector2 position);

public:
    Scene();
    Scene(Vector2 origin, const char *bgHexCode);
    virtual ~Scene() {}

    virtual void initialise() = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual void shutdown() = 0;

    GameState   getState()           const { return mGameState;        }
    Vector2     getOrigin()          const { return mOrigin;           }
    const char *getBGColourHexCode() const { return mBGColourHexCode; }
};

#endif // SCENE_H
