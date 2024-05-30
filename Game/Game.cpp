//
// Created by kevin on 30/05/2024.
//

#include "Game.h"

Game::Game() {
    std::cout << "Start Pong game" << std::endl;
    // Initialize SDL
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    // Setup window
    window = SDL_CreateWindow("Pong", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);

    // Initialize font
    scoreFont = TTF_OpenFont("../Game/DejaVuSansMono.ttf", 40);
    if(scoreFont == NULL){
        std::cout << "Error initializing font: " << TTF_GetError() << std::endl;
    }

    playerOneScoreText = new PlayerScore(Vec2(WINDOW_WIDTH / 4, 20), renderer, scoreFont);
    playerTwoScoreText = new PlayerScore(Vec2(3 * WINDOW_WIDTH / 4, 20), renderer, scoreFont);

    ball = new Ball(
            Vec2((WINDOW_WIDTH / 2.0f) - (BALL_WIDTH / 2.0f),
                 (WINDOW_HEIGHT / 2.0f) - (BALL_WIDTH / 2.0f)),
            Vec2(BALL_SPEED, 0.0f));

    // Construct paddles
    paddleOne = new Paddle(
            Vec2(50.0f, (WINDOW_HEIGHT / 2.0f) - (PADDLE_HEIGHT / 2.0f)),
            Vec2(0.0f, 0.0f));
    paddleTwo = new Paddle(
            Vec2(WINDOW_WIDTH - 50.0f, (WINDOW_HEIGHT / 2.0f) - (PADDLE_HEIGHT / 2.0f)),
            Vec2(0.0f, 0.0f));
}

Game::~Game() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(scoreFont);
    TTF_Quit();
    SDL_Quit();

    delete(playerOneScoreText);
    delete(playerTwoScoreText);
    delete(ball);
    delete(paddleOne);
    delete(paddleTwo);

    std::cout << "Pong game closed" << std::endl;
}

void Game::Run() {
    while (running) {
        auto startTime = std::chrono::high_resolution_clock::now();

        CheckUserInput();

        SetPaddleSpeed();

        UpdateGameObjects();

        CheckCollisions();

        UpdateScreen();

        auto stopTime = std::chrono::high_resolution_clock::now();
        dt = std::chrono::duration<float, std::chrono::milliseconds::period>(stopTime - startTime).count();
    }
}

void Game::CheckUserInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        } else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            } else if (event.key.keysym.sym == SDLK_w) {
                buttons[Buttons::PaddleOneUp] = true;
            } else if (event.key.keysym.sym == SDLK_s) {
                buttons[Buttons::PaddleOneDown] = true;
            } else if (event.key.keysym.sym == SDLK_UP) {
                buttons[Buttons::PaddleTwoUp] = true;
            } else if (event.key.keysym.sym == SDLK_DOWN) {
                buttons[Buttons::PaddleTwoDown] = true;
            }
        } else if (event.type == SDL_KEYUP) {
            if (event.key.keysym.sym == SDLK_w) {
                buttons[Buttons::PaddleOneUp] = false;
            } else if (event.key.keysym.sym == SDLK_s) {
                buttons[Buttons::PaddleOneDown] = false;
            } else if (event.key.keysym.sym == SDLK_UP) {
                buttons[Buttons::PaddleTwoUp] = false;
            } else if (event.key.keysym.sym == SDLK_DOWN) {
                buttons[Buttons::PaddleTwoDown] = false;
            }
        }
    }
}

void Game::SetPaddleSpeed() {
    if (buttons[Buttons::PaddleOneUp]) {
        paddleOne->velocity.y = -PADDLE_SPEED;
    } else if (buttons[Buttons::PaddleOneDown]) {
        paddleOne->velocity.y = PADDLE_SPEED;
    } else {
        paddleOne->velocity.y = 0.0f;
    }

    if (buttons[Buttons::PaddleTwoUp]) {
        paddleTwo->velocity.y = -PADDLE_SPEED;
    } else if (buttons[Buttons::PaddleTwoDown]) {
        paddleTwo->velocity.y = PADDLE_SPEED;
    } else {
        paddleTwo->velocity.y = 0.0f;
    }
}

void Game::UpdateGameObjects() {
    // Update the paddle positions
    paddleOne->Update(dt);
    paddleTwo->Update(dt);

    // Update the ball position
    ball->Update(dt);
}

void Game::CheckCollisions() {
    // Check collisions
    if (Contact contact = CheckPaddleCollision(paddleOne);
            contact.type != CollisionType::None) {
        ball->CollideWithPaddle(contact);
    } else if (contact = CheckPaddleCollision(paddleTwo);
            contact.type != CollisionType::None) {
        ball->CollideWithPaddle(contact);
    } else if (contact = CheckWallCollision();
            contact.type != CollisionType::None) {
        ball->CollideWithWall(contact);

        if (contact.type == CollisionType::Left) {
            ++playerTwoScore;
            playerTwoScoreText->SetScore(playerTwoScore);
        } else if (contact.type == CollisionType::Right) {
            ++playerOneScore;
            playerOneScoreText->SetScore(playerOneScore);
        }
    }
}

void Game::UpdateScreen() {
    SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    for (int y = 0; y < WINDOW_HEIGHT; ++y)
    {
        if (y % 5)
        {
            SDL_RenderDrawPoint(renderer, WINDOW_WIDTH / 2, y);
        }
    }
    // Draw ball
    ball->Draw(renderer);

    // Draw paddles
    paddleOne->Draw(renderer);
    paddleTwo->Draw(renderer);

    // Draw scores
    playerOneScoreText->Draw();
    playerTwoScoreText->Draw();

    SDL_RenderPresent(renderer);
}

Contact Game::CheckPaddleCollision(Paddle* paddle){
    float ballLeft = ball->position.x;
    float ballRight = ball->position.x + BALL_WIDTH;
    float ballTop = ball->position.y;
    float ballBottom = ball->position.y + BALL_HEIGHT;

    float paddleLeft = paddle->position.x;
    float paddleRight = paddle->position.x + PADDLE_WIDTH;
    float paddleTop = paddle->position.y;
    float paddleBottom = paddle->position.y + PADDLE_HEIGHT;

    Contact contact{};

    if (ballLeft >= paddleRight)
    {
        return contact;
    }

    if (ballRight <= paddleLeft)
    {
        return contact;
    }

    if (ballTop >= paddleBottom)
    {
        return contact;
    }

    if (ballBottom <= paddleTop)
    {
        return contact;
    }

    float paddleRangeUpper = paddleBottom - (2.0f * PADDLE_HEIGHT / 3.0f);
    float paddleRangeMiddle = paddleBottom - (PADDLE_HEIGHT / 3.0f);

    if (ball->velocity.x < 0)
    {
        // Left paddle
        contact.penetration = paddleRight - ballLeft;
    }
    else if (ball->velocity.x > 0)
    {
        // Right paddle
        contact.penetration = paddleLeft - ballRight;
    }

    if ((ballBottom > paddleTop)
        && (ballBottom < paddleRangeUpper))
    {
        contact.type = CollisionType::Top;
    }
    else if ((ballBottom > paddleRangeUpper)
             && (ballBottom < paddleRangeMiddle))
    {
        contact.type = CollisionType::Middle;
    }
    else
    {
        contact.type = CollisionType::Bottom;
    }

    return contact;
}

Contact Game::CheckWallCollision(){
    float ballLeft = ball->position.x;
    float ballRight = ball->position.x + BALL_WIDTH;
    float ballTop = ball->position.y;
    float ballBottom = ball->position.y + BALL_HEIGHT;

    Contact contact{};

    if (ballLeft < 0.0f)
    {
        contact.type = CollisionType::Left;
    }
    else if (ballRight > WINDOW_WIDTH)
    {
        contact.type = CollisionType::Right;
    }
    else if (ballTop < 0.0f)
    {
        contact.type = CollisionType::Top;
        contact.penetration = -ballTop;
    }
    else if (ballBottom > WINDOW_HEIGHT)
    {
        contact.type = CollisionType::Bottom;
        contact.penetration = WINDOW_HEIGHT - ballBottom;
    }

    return contact;
}