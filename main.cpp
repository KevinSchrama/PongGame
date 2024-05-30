#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <chrono>

#include "Game/Game.h"

int main() {
    Game pongGame;

    pongGame.Run();

    return 0;
}
