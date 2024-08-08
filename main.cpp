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
int processing;

int hueValue = 40;

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
    VideoCapture cap(-1, CAP_V4L);

    if (!cap.isOpened()){
        mtx.lock();
        cout << "No video stream detected" << endl;
        quit = true;
        mtx.unlock();
        system("pause");
        exit(1);
    }

    auto startLoopTime = chrono::steady_clock::now();
    int i = 0;
    while(!quit && i < 100){
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
        if(debug) cout << "Capture video | " << chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - startTime).count() << endl;
        mtx.unlock();
        // this_thread::sleep_until(startTime + std::chrono::milliseconds(33));
        // struct timespec time = {0,33000000};
        // nanosleep(&time, NULL);
        i++;
    }
    cout << "Mean time per loop for capture: " << (chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - startLoopTime).count())/100 << endl;
    cout << "End video capture" << endl;
    cap.release();
    pthread_exit(NULL);
}

void* processVideo(void *arg){
    cout << "Begin video process" << endl;
    Mat image;
    Mat hsv;
    Mat mask;
    chrono::steady_clock::time_point begin;
    int i = 0;
    int total = 0;
    while(!quit && i < 100){
        if(!readQueue(&queueImage, &image)){
            break;
        }
        auto startLoopTime = chrono::steady_clock::now();
        begin = chrono::steady_clock::now();
        if(image.empty()){
            cout << "empty video" << endl;
            quit = true;
            break;
        }

        flip(image, image, 1);

        cvtColor(image, hsv, COLOR_RGB2HSV);

        Scalar colorLower(hueValue, 100, 90); 
        Scalar colorUpper(90, 240, 255);
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
        addWeighted(image, 0.3, image, 0.0, 0.0, sendPoints.image);

        queuePosition.push(sendPoints);

        if(debug){
            mtx.lock();
            cout << "Process video " << endl;
            cout << "Centroid 1: " << centroid1.y << endl;
            cout << "Centroid 2: " << centroid2.y << endl;
            // cout << "Duration: " << chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - begin).count() << endl;
            mtx.unlock();
        }

        if(processing){
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
        i++;
        total += chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - startLoopTime).count();
    }
    cout << "Mean time per loop for processing: " << (total)/100 << endl;
    cout << "End video process" << endl;
    pthread_exit(NULL);
}

void* setPaddleSpeed(void *arg){
    Game* game = (Game*) arg;
    // start in the middle of the screen
    controlPoints newPositions;
    newPositions.centroid1x = 0;
    newPositions.centroid1y = WINDOW_HEIGHT/2;
    newPositions.centroid2x = 0;
    newPositions.centroid2y = WINDOW_HEIGHT/2;
    while(!quit){
        if(pollQueue(&queuePosition, &newPositions)){
            mtx.lock();
            if(debug) cout << "New position" << endl;
            mtx.unlock();
        }
        mtxGame.lock();
        game->SetPaddleSpeed(newPositions);
        mtxGame.unlock();
        this_thread::sleep_for (std::chrono::milliseconds(1));
    }
    pthread_exit(NULL);
}

void* updateObjects(void *arg){
    Game* game = (Game*) arg;
    while(!quit){
        mtx.lock();
        if(debug) cout << "Update objects" << endl;
        mtx.unlock();
        mtxGame.lock();
        game->UpdateGameObjects();
        mtxGame.unlock();
        queueToCollisionCheck.push(false);
        this_thread::sleep_for (std::chrono::milliseconds(33));
    }
    pthread_exit(NULL);
}

void* checkCollisions(void *arg){
    Game* game = (Game*) arg;
    while(!quit){
        if(!readQueue(&queueToCollisionCheck, (bool*)nullptr)){
            break;
        }
        mtx.lock();
        if(debug) cout << "Check collision" << endl;
        mtx.unlock();
        mtxGame.lock();
        game->CheckCollisions();
        mtxGame.unlock();
        queueToUpdateScreen.push(false);
    }
    pthread_exit(NULL);
}

void* updateScreen(void *arg){
    Game* game = (Game*) arg;
    auto startTime = std::chrono::high_resolution_clock::now();
    auto stopTime = std::chrono::high_resolution_clock::now();
    while(!quit){
        if(!readQueue(&queueToUpdateScreen, (bool*)nullptr)){
            break;
        }
        mtx.lock();
        if(debug) cout << "Update screen" << endl;
        mtx.unlock();
        mtxGame.lock();
        game->UpdateScreen();
        mtxGame.unlock();
        stopTime = std::chrono::high_resolution_clock::now();
        mtxGame.lock();
        game->SetTimeDelta(std::chrono::duration<float, std::chrono::milliseconds::period>(stopTime - startTime).count());
        mtxGame.unlock();
        startTime = std::chrono::high_resolution_clock::now();
    }
    pthread_exit(NULL);
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

    cout << "Start rendering loop" << endl;
    int i = 0;
    int total = 0;
    while(!quit && i < 100){
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
        auto startLoopTime = chrono::steady_clock::now();
        if(pollQueue(&queuePosition, &newPositions)){
            mtx.lock();
            if(debug) cout << "New position" << endl;
            mtx.unlock();
        }
        game.SetPaddleSpeed(newPositions);
        game.UpdateGameObjects();
        game.CheckCollisions();
        game.UpdateScreen();
        total += chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - startLoopTime).count();
        mtx.lock();
        if(debug) cout << "Update Game " << time << endl;
        mtx.unlock();
        // this_thread::sleep_until(startTime + std::chrono::milliseconds(200));
        struct timespec timesleep = { 0, 33000000 };
        nanosleep(&timesleep, NULL);
        stopTime = std::chrono::high_resolution_clock::now();
        static float time = std::chrono::duration<float, std::chrono::milliseconds::period>(stopTime - startTime).count();
        static float dt = 33.0f;
        game.SetTimeDelta(time);
        i++;
    }
    cout << "Mean time per loop for game rendering: " << (total)/100 << endl;
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    quit = false;
    debug = false;
    bool gameOnly = false;
    bool divided = false;
    pthread_t thread1;
    pthread_t thread2;
    pthread_t thread3;

    pthread_t paddleSpeedThread;
    pthread_t updateObjectsThread;
    pthread_t checkCollisionsThread;
    pthread_t updateScreenThread;

    void* ret;

    if(argc > 1){
        if(strcmp(argv[1], "-d") == 0){
            cout << "Debug on" << endl;
            debug = true;
        }
        if(strcmp(argv[1], "-g") == 0){
            gameOnly = true;
        }
        if(strcmp(argv[1], "--div") == 0){
            divided = true;
        }
        if(strcmp(argv[1], "-p") == 0){
            cout << "Show only video processing" << endl;
            processing = true;
        }
        if(argc > 2){
            hueValue = atoi(argv[2]);
        }
    }
    
    cout << "Start threads" << endl;

    if(!gameOnly){
        if (pthread_create(&thread1, NULL, captureVideo, NULL) != 0) {
            perror("pthread_create() error 1");
            exit(1);
        }
        if (pthread_create(&thread2, NULL, processVideo, NULL) != 0) {
            perror("pthread_create() error 2");
            exit(1);
        }
    }
    if(!processing){
        if(!divided){
            if (pthread_create(&thread3, NULL, updateGame, NULL) != 0) {
                perror("pthread_create() error 3");
                exit(1);
            }
        }else{
            static Game game;
            if (pthread_create(&paddleSpeedThread, NULL, setPaddleSpeed, &game) != 0) {
                perror("pthread_create() error paddle speed");
                exit(1);
            }
            if (pthread_create(&updateObjectsThread, NULL, updateObjects, &game) != 0) {
                perror("pthread_create() error update objects");
                exit(1);
            }
            if (pthread_create(&checkCollisionsThread, NULL, checkCollisions, &game) != 0) {
                perror("pthread_create() error check collisions");
                exit(1);
            }
            if (pthread_create(&updateScreenThread, NULL, updateScreen, &game) != 0) {
                perror("pthread_create() error check collisions");
                exit(1);
            }
        }
    }

    cout << "Set priority" << endl;

    if(!gameOnly)pthread_setschedprio(thread1, 1);
    if(!gameOnly)pthread_setschedprio(thread2, 1);
    if(!processing){
        if(!divided){
            pthread_setschedprio(thread3, 1);
        }else{
            pthread_setschedprio(paddleSpeedThread, 1);
            pthread_setschedprio(updateObjectsThread, 1);
            pthread_setschedprio(checkCollisionsThread, 1);
            pthread_setschedprio(updateScreenThread, 1);
        }
    }

    if(!gameOnly){
        if (pthread_join(thread1, &ret) != 0) {
            perror("pthread_join() error");
            exit(3);
        }
        if (pthread_join(thread2, &ret) != 0) {
            perror("pthread_join() error");
            exit(3);
        }
    }
    if(!processing){
        if(!divided){
            if (pthread_join(thread3, &ret) != 0) {
                perror("pthread_join() error");
                exit(3);
            }
        }else{
            SDL_Event event;
            while(!quit){
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) {
                        quit = true;
                    }else if (event.type == SDL_KEYDOWN) {
                        if (event.key.keysym.sym == SDLK_ESCAPE) {
                            quit = true;
                        }
                    }
                }
                struct timespec timesleep = { 0, 11000000 };
                nanosleep(&timesleep, NULL);
            }
            if (pthread_join(paddleSpeedThread, &ret) != 0) {
                perror("pthread_join() error paddle speed");
                exit(1);
            }
            if (pthread_join(updateObjectsThread, &ret) != 0) {
                perror("pthread_join() error update objects");
                exit(1);
            }
            if (pthread_join(checkCollisionsThread, &ret) != 0) {
                perror("pthread_join() error check collisions");
                exit(1);
            }
            if (pthread_join(updateScreenThread, &ret) != 0) {
                perror("pthread_join() error check collisions");
                exit(1);
            }
        }
    }

    return 0;
}