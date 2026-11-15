#ifndef ENDSCREEN_H
#define ENDSCREEN_H

#include "Scene.h"

// used for both the win and the lose screen
class EndScreen : public Scene
{
private:
    const char *mMessageFilepath;

    Texture2D mMessageTexture;
    Texture2D mPromptTexture;

public:
    EndScreen();
    EndScreen(Vector2 origin, const char *bgHexCode, const char *messageFilepath);
    ~EndScreen();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif // ENDSCREEN_H
