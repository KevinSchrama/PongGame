//
// Created by kevin on 17/05/2024.
//

#ifndef PONGGAME_PLAYERSCORE_H
#define PONGGAME_PLAYERSCORE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "Vec2.h"
#include "constants.h"

#include <iostream>

class PlayerScore {
public:
    PlayerScore(Vec2 position, SDL_Renderer* renderer, TTF_Font* font);

    ~PlayerScore();

    void SetScore(int score);

    void Draw();

    SDL_Renderer* renderer;
    TTF_Font* font;
    SDL_Surface* surface{};
    SDL_Texture* texture{};
    SDL_Rect rect{};
};


#endif //PONGGAME_PLAYERSCORE_H
