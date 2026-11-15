/**
* Author: Deyon Wijewardana
* Assignment: Rise of the AI
* Date due: 11/16/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "EndScreen.h"

EndScreen::EndScreen()
    : Scene{ { 0.0f, 0.0f }, "#000000" }, mMessageFilepath{ "" }
{}

EndScreen::EndScreen(Vector2 origin, const char *bgHexCode, const char *messageFilepath)
    : Scene{ origin, bgHexCode }, mMessageFilepath{ messageFilepath }
{}

EndScreen::~EndScreen() { shutdown(); }

void EndScreen::initialise()
{
    mGameState.nextSceneID = NO_SCENE;
    mGameState.playerDied  = false;
    mGameState.player      = nullptr;
    mGameState.map         = nullptr;
    mGameState.enemyCount  = 0;

    mMessageTexture = LoadTexture(mMessageFilepath);
    mPromptTexture  = LoadTexture("assets/back_to_menu.png");
}

// ENTER is handled in processInput() in main.cpp
void EndScreen::update(float deltaTime) {}

void EndScreen::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    drawCentred(mMessageTexture, { mOrigin.x, mOrigin.y - 60.0f  });
    drawCentred(mPromptTexture,  { mOrigin.x, mOrigin.y + 130.0f });
}

void EndScreen::shutdown()
{
    UnloadTexture(mMessageTexture);
    UnloadTexture(mPromptTexture);

    mMessageTexture = {};
    mPromptTexture  = {};
}
