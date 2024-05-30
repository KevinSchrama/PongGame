//
// Created by kevin on 17/05/2024.
//

#ifndef PONGGAME_BALL_H
#define PONGGAME_BALL_H

#include <SDL2/SDL.h>
#include "Vec2.h"
#include "constants.h"

class Ball {
public:
    Ball(Vec2 position, Vec2 velocity);

    void Update(float dt);

    void CollideWithPaddle(Contact const& contact);

    void CollideWithWall(Contact const& contact);

    void Draw(SDL_Renderer* renderer);

    Vec2 position;
    Vec2 velocity;
    SDL_Rect rect{};
};


#endif //PONGGAME_BALL_H
