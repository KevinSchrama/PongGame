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
    Mat image;
    VideoCapture cap(0);//Declaring an object to capture stream of frames from default camera//
    if (!cap.isOpened()){ //This section prompt an error message if no video stream is found//
        std::cout << "No video stream detected" << std::endl;
        system("pause");
        return-1;
    }

    while (true){ //Taking an everlasting loop to show the video//
        cap >> image;
        if (image.empty()){ //Breaking the loop if no video frame is detected//
            break;
        }
        imshow("Video Player", image);//Showing the video//
        char c = (char)waitKey(25);//Allowing 25 milliseconds frame processing time and initiating break condition//
        if (c == 27){ //If 'Esc' is entered break the loop//
            break;
        }
    }
    cap.release();


    return 0;
}
