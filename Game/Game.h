//
// Created by kevin on 30/05/2024.
//

#ifndef PONGGAME_GAME_H
#define PONGGAME_GAME_H

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <chrono>

#include "Vec2.h"
#include "Ball.h"
#include "Paddle.h"
#include "PlayerScore.h"
#include "constants.h"


class Game {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;

    TTF_Font* scoreFont;

    PlayerScore* playerOneScoreText;
    PlayerScore* playerTwoScoreText;

    Ball* ball;
    Paddle* paddleOne;
    Paddle* paddleTwo;

    int playerOneScore = 0;
    int playerTwoScore = 0;

    bool running = true;
    bool buttons[4] = {};

    float dt = 0.0f;

    void CheckUserInput();

    void SetPaddleSpeed();

    void UpdateGameObjects();

    void CheckCollisions();

    void UpdateScreen();

    Contact CheckPaddleCollision(Paddle* paddle);

    Contact CheckWallCollision();

public:
    Game();

    ~Game();

    void Run();

};


#endif //PONGGAME_GAME_H
