#ifndef MENU_H
#define MENU_H

#include "Scene.h"

class Menu : public Scene
{
private:
    // all the text is drawn as images
    Texture2D mTitleTexture;
    Texture2D mPromptTexture;
    Texture2D mControlsTexture;
    Texture2D mCharactersTexture;

public:
    Menu();
    Menu(Vector2 origin, const char *bgHexCode);
    ~Menu();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif // MENU_H
