//
// Created by kevin on 17/05/2024.
//

#ifndef PONGGAME_INDICATOR_H
#define PONGGAME_INDICATOR_H

#include <SDL2/SDL.h>
#include "Vec2.h"
#include "constants.h"

class Indicator {
public:
    Indicator(Vec2 position);

    void Update(int x, int y);

    void Draw(SDL_Renderer* renderer);

    Vec2 position;
    SDL_Rect rect{};
};


#endif //PONGGAME_INDICATOR_H
