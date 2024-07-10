#include <iostream>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <thread>
#include <mutex>
#include <queue>

#include <time.h>
#include <stdio.h>
#include <pthread.h>

#include "Game/Game.h"
#include "FixedQueue.h"

using namespace cv;
using namespace std;

mutex mtx;
mutex mtxGame;

FixedQueue<Mat, 1> queueImage;
FixedQueue<controlPoints, 1> queuePosition;
FixedQueue<bool, 1> queueToCollisionCheck;
FixedQueue<bool, 1> queueToUpdateScreen;

int quit;
int debug;

int hueValue = 50;

bool readQueue(queue<auto>* queueRead, auto* out){
    while(queueRead->empty()){
        if(quit){
            return false;
        }
        this_thread::sleep_for (std::chrono::milliseconds(1));
    }
    if(out){
        *out = queueRead->front();
    }
    queueRead->pop();
    return true;
}

bool pollQueue(queue<auto>* queuePoll, auto* out){
    if(queuePoll->empty()){
        return false;
    }
    *out = queuePoll->front();
    queuePoll->pop();
    return true;
}

void* captureVideo(void *arg){
    cout << "Begin video capture" << endl;
    Mat image;
    VideoCapture cap(0, CAP_V4L);

    if (!cap.isOpened()){
        mtx.lock();
        cout << "No video stream detected" << endl;
        quit = true;
        mtx.unlock();
        system("pause");
        exit(1);
    }

    while(!quit){
        auto startTime = chrono::steady_clock::now();
        cap >> image;
        Mat smaller;
        resize(image, smaller, Size(1280, 720), INTER_LINEAR);
        while(!queueImage.check_space() && !quit){
            struct timespec time = {0,1000000};
            nanosleep(&time, NULL);
        }
        queueImage.push(smaller);
        mtx.lock();
        cout << "Capture video | " << chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - startTime).count() << endl;
        mtx.unlock();
        // this_thread::sleep_until(startTime + std::chrono::milliseconds(33));
        // struct timespec time = {0,33000000};
        // nanosleep(&time, NULL);
    }

    cout << "End video capture" << endl;
    cap.release();
    char* ret;
    strcpy(ret, "This is a test");
    pthread_exit(ret);
}

void* processVideo(void *arg){
    cout << "Begin video process" << endl;
    Mat image;
    Mat hsv;
    Mat mask;
    chrono::steady_clock::time_point begin;
    while(!quit){
        if(!readQueue(&queueImage, &image)){
            break;
        }
        begin = chrono::steady_clock::now();
        if(image.empty()){
            cout << "empty video" << endl;
            quit = true;
            break;
        }

        flip(image, image, 1);

        cvtColor(image, hsv, COLOR_RGB2HSV);

        Scalar colorLower(hueValue, 80, 80); 
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
        Rect rectBig(-1, -1, -1, -1);
        if (maxAreaIdx != -1) {
            rectBig = boundingRect(contours[maxAreaIdx]);
            // centroid1 = Point2f(rectBig.x+(rectBig.width/2), rectBig.y+(rectBig.height/2));
            Moments M1 = moments(contours[maxAreaIdx]);
            centroid1 = Point2f(M1.m10 / M1.m00, M1.m01 / M1.m00);
        }

        // Find center of second biggest contour
        Point2f centroid2(-1, -1);
        Rect rectSmall(-1, -1, -1, -1);
        if (secondAreaIdx != -1) {
            rectSmall = boundingRect(contours[secondAreaIdx]);
            // centroid1 = Point2f(rectSmall.x+(rectSmall.width/2), rectSmall.y+(rectSmall.height/2));
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
        cout << "Process video " << endl;
        cout << "Centroid 1: " << centroid1.y << endl;
        cout << "Centroid 2: " << centroid2.y << endl;
        // cout << "Duration: " << chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - begin).count() << endl;
        mtx.unlock();

        if(debug){
            Mat imageSmall;
            cvtColor(mask, imageSmall, COLOR_GRAY2RGB);
            double a = 0.5;
            double b = 1.0 - a;
            addWeighted(imageSmall, a, image, b, 0.0, imageSmall);
            drawContours(imageSmall, contours, -1, Scalar(255, 0, 0), 5);
            rectangle(imageSmall, rectBig, Scalar(0, 0, 255), 5);
            rectangle(imageSmall, rectSmall, Scalar(0, 0, 255), 5);
            circle(imageSmall, centroid1, 10, Scalar(0, 0, 255), -1);
            circle(imageSmall, centroid2, 10, Scalar(0, 0, 255), -1);
            resize(imageSmall, imageSmall, Size(640, 360), INTER_LINEAR);
            imshow("Image", imageSmall);
            char c=(char)waitKey(25);
            if(c==27){
                quit = true;
                break;
            }
        }
    }
    cout << "End video process" << endl;
    char* ret;
    strcpy(ret, "This is a test");
    pthread_exit(ret);
}

void setPaddleSpeed(Game& game){
    // start in the middle of the screen
    controlPoints newPositions;
    newPositions.centroid1x = 0;
    newPositions.centroid1y = WINDOW_HEIGHT/2;
    newPositions.centroid2x = 0;
    newPositions.centroid2y = WINDOW_HEIGHT/2;
    while(!quit){
        if(pollQueue(&queuePosition, &newPositions)){
            mtx.lock();
            cout << "New position" << endl;
            mtx.unlock();
        }
        mtxGame.lock();
        game.SetPaddleSpeed(newPositions);
        mtxGame.unlock();
        this_thread::sleep_for (std::chrono::milliseconds(1));
    }
}

void updateObjects(Game& game){
    SDL_Event event;
    while(!quit){
        mtxGame.lock();
        game.UpdateGameObjects();
        mtxGame.unlock();
        queueToCollisionCheck.push(false);
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                }
            }
        }
        this_thread::sleep_for (std::chrono::milliseconds(25));
    }
}

void checkCollisions(Game& game){
    while(!quit){
        if(!readQueue(&queueToCollisionCheck, (bool*)nullptr)){
            break;
        }
        cout << "Check collision" << endl;
        game.CheckCollisions();
        queueToUpdateScreen.push(false);
    }
}

void updateScreen(Game& game){
    auto startTime = std::chrono::high_resolution_clock::now();
    auto stopTime = std::chrono::high_resolution_clock::now();
    while(!quit){
        if(!readQueue(&queueToUpdateScreen, (bool*)nullptr)){
            break;
        }
        cout << "Update screen" << endl;
        game.UpdateScreen();
        stopTime = std::chrono::high_resolution_clock::now();
        game.SetTimeDelta(std::chrono::duration<float, std::chrono::milliseconds::period>(stopTime - startTime).count());
        startTime = std::chrono::high_resolution_clock::now();
    }
}

void* updateGame(void *arg){
    Game game;
    SDL_Event event;
    auto startTime = std::chrono::high_resolution_clock::now();
    auto stopTime = std::chrono::high_resolution_clock::now();

    controlPoints newPositions;
    newPositions.centroid1x = 0;
    newPositions.centroid1y = WINDOW_HEIGHT/2;
    newPositions.centroid2x = 0;
    newPositions.centroid2y = WINDOW_HEIGHT/2;

    while(!quit){
        startTime = std::chrono::high_resolution_clock::now();
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    quit = true;
                }
            }
        }
        if(pollQueue(&queuePosition, &newPositions)){
            mtx.lock();
            cout << "New position" << endl;
            mtx.unlock();
        }
        game.SetPaddleSpeed(newPositions);
        game.UpdateGameObjects();
        game.CheckCollisions();
        game.UpdateScreen();
        stopTime = std::chrono::high_resolution_clock::now();
        static float time = std::chrono::duration<float, std::chrono::milliseconds::period>(stopTime - startTime).count();
        static float dt = 100.0f;
        game.SetTimeDelta(time);
        mtx.lock();
        cout << "Update Game " << time << endl;
        mtx.unlock();
        // this_thread::sleep_until(startTime + std::chrono::milliseconds(200));
        // struct timespec timesleep = { 0, 100000000 };
        // nanosleep(&timesleep, NULL);
    }
    char* ret;
    strcpy(ret, "This is a test");
    pthread_exit(ret);
}


/*int main(int argc, char *argv[]) {
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
    // thread setPaddleSpeedThread(setPaddleSpeed, ref(game));
    thread updateGameThread(updateGame);

    // thread updateObjectsThread(updateObjects, ref(game));
    // thread checkCollisionsThread(checkCollisions, ref(game));
    // thread updateScreenThread(updateScreen, ref(game));

    processVideoThread.join();
    videoCaptureThread.join();
    // setPaddleSpeedThread.join();
    updateGameThread.join();

    // updateObjectsThread.join();
    // checkCollisionsThread.join();
    // updateScreenThread.join();

    return 0;
}*/

int main(int argc, char *argv[]) {
    quit = false;
    debug = false;
    pthread_t thread1;
    pthread_t thread2;
    pthread_t thread3;
    void* ret;

    if(argc > 1){
        if(strcmp(argv[1], "-d") == 0){
            cout << "Debug on" << endl;
            debug = true;
        }
        if(argc > 2){
            hueValue = atoi(argv[2]);
        }
    }
    
    cout << "Start threads" << endl;

    if (pthread_create(&thread1, NULL, captureVideo, NULL) != 0) {
        perror("pthread_create() error 1");
        exit(1);
    }
    if (pthread_create(&thread2, NULL, processVideo, NULL) != 0) {
        perror("pthread_create() error 2");
        exit(1);
    }
    if(!debug){
        if (pthread_create(&thread3, NULL, updateGame, NULL) != 0) {
            perror("pthread_create() error 3");
            exit(1);
        }
    }

    cout << "Set priority" << endl;

    pthread_setschedprio(thread1, 1);
    pthread_setschedprio(thread2, 1);
    if(!debug) pthread_setschedprio(thread3, 1);

    if (pthread_join(thread1, &ret) != 0) {
        perror("pthread_join() error");
        exit(3);
    }
    if (pthread_join(thread2, &ret) != 0) {
        perror("pthread_join() error");
        exit(3);
    }
    if(!debug){
        if (pthread_join(thread3, &ret) != 0) {
            perror("pthread_join() error");
            exit(3);
        }
    }

    return 0;
}