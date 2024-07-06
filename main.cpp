#include <iostream>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <thread>
#include <mutex>
#include <queue>
#include <chrono>

#include "Game/Game.h"

using namespace cv;
using namespace std;

struct controlPoints {
    int centroid1x;
    int centroid1y;
    int centroid2x;
    int centroid2y;
};

/*
int main() {
    // Game pongGame;

    // pongGame.Run();

    // return 0;

    SDL_Window* window;
    SDL_Renderer* renderer;

    bool running = true;

    window = SDL_CreateWindow("Pong", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);

    Mat image;
    Mat hsv;
    VideoCapture cap(0);//Declaring an object to capture stream of frames from default camera//
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

        cv::imshow("image", mask);

        // SDL_Texture* tex = SDL_CreateTexture(
        //         renderer, SDL_PIXELFORMAT_BGR24, SDL_TEXTUREACCESS_STREAMING, mask.cols,
        //         mask.rows);
        // SDL_UpdateTexture(tex, NULL, (void*)mask.data, mask.step1());

        // SDL_Rect texture_rect;
        // texture_rect.x = 0; //the x coordinate
        // texture_rect.y = 0; //the y coordinate
        // texture_rect.w = WINDOW_WIDTH; //the width of the texture
        // texture_rect.h = WINDOW_HEIGHT; //the height of the texture

        // SDL_RenderClear(renderer);
        // SDL_RenderCopy(renderer, tex, NULL, &texture_rect);
        // SDL_RenderPresent(renderer);

    }
    cap.release();


    return 0;
}*/

mutex mtx;
queue<Mat> queueImage;
queue<controlPoints> queuePosition;
int quit;
int debug;

auto readQueue(queue<auto>* queueRead){
    while(queueRead->empty()){
        usleep(1000);
    }
    auto out = queueRead->front();
    queueRead->pop();
    return out;
}

void captureVideo(){
    Mat image;
    VideoCapture cap(0);

    if (!cap.isOpened()){
        mtx.lock();
        cout << "No video stream detected" << endl;
        quit = true;
        mtx.unlock();
        system("pause");
        return;
    }

    while(!quit){
        cap >> image;
        Mat smaller;
        resize(image, smaller, Size(1280, 720), INTER_LINEAR);
        mtx.lock();
        cout << "Capture video | " << smaller.cols << "x" << smaller.rows << endl;
        mtx.unlock();
        queueImage.push(smaller);
        usleep(33000);
    }

    cap.release();
}

void processVideo(){
    Mat image;
    Mat hsv;
    Mat mask;
    chrono::steady_clock::time_point begin;
    while(!quit){
        image = readQueue(&queueImage);
        begin = chrono::steady_clock::now();
        if(image.empty()){
            cout << "empty video" << endl;
            quit = true;
            break;
        }

        cvtColor(image, hsv, COLOR_RGB2HSV);

        Scalar colorLower(60, 50, 50); 
        Scalar colorUpper(90, 255, 255);
        inRange(hsv, colorLower, colorUpper, mask);

        vector<vector<Point>> contours;
        findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        double maxArea = 0;
        int maxAreaIdx = -1;
        int secondAreaIdx = -1;
        for (size_t i = 0; i < contours.size(); i++) {
            double area = contourArea(contours[i]);
            if (area > maxArea) {
                maxArea = area;
                secondAreaIdx = maxAreaIdx;
                maxAreaIdx = i;
            }
        }

        // Find center of biggest contour
        Point2f centroid1(-1, -1);
        if (maxAreaIdx != -1) {
            Moments M1 = moments(contours[maxAreaIdx]);
            centroid1 = Point2f(M1.m10 / M1.m00, M1.m01 / M1.m00);
        }

        // Find center of second biggest contour
        Point2f centroid2(-1, -1);
        if (secondAreaIdx != -1) {
            Moments M2 = moments(contours[secondAreaIdx]);
            centroid2 = Point2f(M2.m10 / M2.m00, M2.m01 / M2.m00);
        }

        controlPoints sendPoints;
        sendPoints.centroid1x = centroid1.x;
        sendPoints.centroid1y = centroid1.y;
        sendPoints.centroid2x = centroid2.x;
        sendPoints.centroid2y = centroid2.y;

        queuePosition.push(sendPoints);

        mtx.lock();
        cout << "Process video " << centroid1.x << " - " << centroid1.y << endl;
        mtx.unlock();

        cout << "Duration: " << chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - begin).count() << endl;

        if(debug){
            Mat imageSmall;
            circle(image, centroid1, 5, Scalar(0, 0, 255), -1);
            circle(image, centroid2, 5, Scalar(0, 0, 255), -1);
            resize(image, imageSmall, Size(640, 360), INTER_LINEAR);
            imshow("Image", imageSmall);
            char c=(char)waitKey(25);
            if(c==27){
                quit = true;
                break;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    quit = false;
    debug = false;

    if(argc > 1){
        if(strcmp(argv[1], "-d") == 0){
            cout << "Debug on" << endl;
            debug = true;
        }
    }

    thread videoCaptureThread(captureVideo);
    thread processVideoThread(processVideo);

    processVideoThread.join();
    videoCaptureThread.join();

    return 0;
}