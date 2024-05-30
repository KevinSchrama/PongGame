//
// Created by kevin on 17/05/2024.
//

#ifndef PONGGAME_CONSTANTS_H
#define PONGGAME_CONSTANTS_H

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

const int PADDLE_WIDTH = 10;
const int PADDLE_HEIGHT = 100;

const float PADDLE_SPEED = 0.5f;

const int BALL_WIDTH = 15;
const int BALL_HEIGHT = 15;

const float BALL_SPEED = 0.5f;

enum Buttons
{
    PaddleOneUp = 0,
    PaddleOneDown,
    PaddleTwoUp,
    PaddleTwoDown,
};

enum class CollisionType
{
    None,
    Top,
    Middle,
    Bottom,
    Left,
    Right
};

struct Contact
{
    CollisionType type;
    float penetration;
};


#endif //PONGGAME_CONSTANTS_H
