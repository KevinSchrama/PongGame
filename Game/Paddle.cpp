//
// Created by kevin on 17/05/2024.
//

#include "Paddle.h"

Paddle::Paddle(Vec2 position, Vec2 velocity) : position(position), velocity(velocity){
    rect.x = static_cast<int>(position.x);
    rect.y = static_cast<int>(position.y);
    rect.w = PADDLE_WIDTH;
    rect.h = PADDLE_HEIGHT;
}

void Paddle::Draw(SDL_Renderer *renderer) {
    rect.y = static_cast<int>(position.y);

    SDL_RenderFillRect(renderer, &rect);
}

void Paddle::Update(float dt) {
    position += velocity * dt;

    if (position.y < 0){
        position.y = 0;
    } else if (position.y > (WINDOW_HEIGHT - PADDLE_HEIGHT)){
        position.y = WINDOW_HEIGHT - PADDLE_HEIGHT;
    }
}

void Paddle::SetSpeed(int newPosition){
    if(newPosition >= 0){
        velocity.y = ((float)newPosition - position.y) * PADDLE_DIF;
    }
}