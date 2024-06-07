#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <chrono>
#include <opencv2/opencv.hpp>

#include "Game/Game.h"

using namespace cv;

int main() {
//    Game pongGame;
//
//    pongGame.Run();

    SDL_Window* window;
    SDL_Renderer* renderer;

    bool running = true;

    window = SDL_CreateWindow("Pong", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);

    Mat image;
    Mat hsv;
    VideoCapture cap(1);//Declaring an object to capture stream of frames from default camera//
    if (!cap.isOpened()){ //This section prompt an error message if no video stream is found//
        std::cout << "No video stream detected" << std::endl;
        system("pause");
        return-1;
    }

    while (running){ //Taking an everlasting loop to show the video//
        cap >> image;
        if (image.empty()) { //Breaking the loop if no video frame is detected//
            break;
        }
        cvtColor(image, hsv, COLOR_RGB2HSV);

        // Define the lower and upper bounds for the color you want to detect (in HSV)
        cv::Scalar colorLower(60, 50, 50); // Example: lower bound for blue color
        cv::Scalar colorUpper(90, 255, 255); // Example: upper bound for blue color

        // Mask the image to get only the specified color
        cv::Mat mask;
        cv::inRange(hsv, colorLower, colorUpper, mask);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        SDL_Texture* tex = SDL_CreateTexture(
                renderer, SDL_PIXELFORMAT_BGR24, SDL_TEXTUREACCESS_STREAMING, mask.cols,
                mask.rows);
        SDL_UpdateTexture(tex, NULL, (void*)mask.data, mask.step1());

        SDL_Rect texture_rect;
        texture_rect.x = 0; //the x coordinate
        texture_rect.y = 0; //the y coordinate
        texture_rect.w = WINDOW_WIDTH; //the width of the texture
        texture_rect.h = WINDOW_HEIGHT; //the height of the texture

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, tex, NULL, &texture_rect);
        SDL_RenderPresent(renderer);

    }
    cap.release();


    return 0;
}
