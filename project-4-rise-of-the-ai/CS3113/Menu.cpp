/**
* Author: Deyon Wijewardana
* Assignment: Rise of the AI
* Date due: 11/16/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "Menu.h"

Menu::Menu()
    : Scene{ { 0.0f, 0.0f }, "#000000" }
{}

Menu::Menu(Vector2 origin, const char *bgHexCode)
    : Scene{ origin, bgHexCode }
{}

Menu::~Menu() { shutdown(); }

void Menu::initialise()
{
    mGameState.nextSceneID = NO_SCENE;
    mGameState.playerDied  = false;
    mGameState.player      = nullptr;
    mGameState.map         = nullptr;
    mGameState.enemyCount  = 0;

    mTitleTexture      = LoadTexture("assets/title.png");
    mPromptTexture     = LoadTexture("assets/press_enter.png");
    mControlsTexture   = LoadTexture("assets/controls.png");
    mCharactersTexture = LoadTexture("assets/characters.png");

    mGameState.bgm = LoadMusicStream("assets/menu_music.mp3");
    SetMusicVolume(mGameState.bgm, 0.25f);
    PlayMusicStream(mGameState.bgm);
}

// ENTER is handled in processInput() in main.cpp
void Menu::update(float deltaTime)
{
    UpdateMusicStream(mGameState.bgm);
}

void Menu::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    drawCentred(mTitleTexture,    { mOrigin.x, mOrigin.y - 170.0f });
    drawCentred(mControlsTexture, { mOrigin.x, mOrigin.y + 35.0f  });
    drawCentred(mPromptTexture,   { mOrigin.x, mOrigin.y + 190.0f });

    // astronaut on the left, robots on the right
    int   frames[4] = { 0, 15, 21, 24 };
    float xs[4]     = { mOrigin.x - 360.0f, mOrigin.x + 260.0f, mOrigin.x + 340.0f, mOrigin.x + 420.0f };

    for (int i = 0; i < 4; i++)
    {
        Rectangle textureArea     = getUVRectangle(&mCharactersTexture, frames[i], 3, 9);
        Rectangle destinationArea = { xs[i], mOrigin.y + 35.0f, 78.0f, 78.0f };

        DrawTexturePro(mCharactersTexture, textureArea, destinationArea, { 39.0f, 39.0f }, 0.0f, WHITE);
    }
}

void Menu::shutdown()
{
    UnloadTexture(mTitleTexture);
    UnloadTexture(mPromptTexture);
    UnloadTexture(mControlsTexture);
    UnloadTexture(mCharactersTexture);

    mTitleTexture      = {};
    mPromptTexture     = {};
    mControlsTexture   = {};
    mCharactersTexture = {};

    UnloadMusicStream(mGameState.bgm);
    mGameState.bgm = {};
}
