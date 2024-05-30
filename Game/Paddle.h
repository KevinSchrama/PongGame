//
// Created by kevin on 17/05/2024.
//

#ifndef PONGGAME_PADDLE_H
#define PONGGAME_PADDLE_H

#include <SDL2/SDL.h>
#include "Vec2.h"
#include "constants.h"

class Paddle {
public:
    Paddle(Vec2 position, Vec2 velocity);

    void Draw(SDL_Renderer* renderer);

    void Update(float dt);

    Vec2 position;
    Vec2 velocity;
    SDL_Rect rect{};
};


#endif //PONGGAME_PADDLE_H
