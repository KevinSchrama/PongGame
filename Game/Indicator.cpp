//
// Created by kevin on 08/07/2024.
//

#include "Indicator.h"

Indicator::Indicator(Vec2 position) : position(position) {
    rect.x = static_cast<int>(position.x);
    rect.y = static_cast<int>(position.y);
    rect.w = INDICATOR_WIDTH;
    rect.h = INDICATOR_HEIGHT;
}

void Indicator::Update(int x, int y) {
    position.x = (float)x;
    position.y = (float)y;
}

void Indicator::Draw(SDL_Renderer* renderer) {
    rect.x = static_cast<int>(position.x);
    rect.y = static_cast<int>(position.y);

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
}