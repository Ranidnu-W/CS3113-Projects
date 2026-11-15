#ifndef LEVELD_H
#define LEVELD_H

#include "Scene.h"

// boss level (extra credit)
class LevelD : public Scene
{
public:
    static constexpr int LEVEL_WIDTH  = 26,
                         LEVEL_HEIGHT = 11,
                         BOSS_HEALTH  = 3;

    static constexpr float TILE_DIMENSION          = 54.0f,
                           ACCELERATION_OF_GRAVITY = 981.0f,
                           FALL_DEATH_DISTANCE     = 200.0f,
                           STUN_TIME               = 1.5f;

private:
    // 0 = empty, otherwise the tile number in tileset.png + 1
    unsigned int mLevelData[LEVEL_WIDTH * LEVEL_HEIGHT] = {
        121,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 121,
        121,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 121,
        121,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 121,
        121,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 121,
        121,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   2,   3,   3,   4,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 121,
        121,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 121,
        121,   0,   0,   0,   2,   3,   4,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   2,   3,   4,   0,   0,   0, 121,
        121,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 121,
        121,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 121,
        121,  22,  23,  23,  23,  23,  23,  23,  23,  23,  23,  23,  23,  23,  23,  23,  23,  23,  23,  23,  23,  23,  23,  23,  24, 121,
        121, 142, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 143, 144, 121
    };

    int   mBossHealth = BOSS_HEALTH;
    float mStunTimer  = 0.0f;

    Sound     mStompSound;
    Texture2D mHintTexture;

public:
    LevelD();
    LevelD(Vector2 origin, const char *bgHexCode);
    ~LevelD();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif // LEVELD_H
