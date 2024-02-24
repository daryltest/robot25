// g++ -Wall -pthread -o prog *.cpp -lpigpio -lrt

#include <iostream>
#include <unistd.h>
#include <math.h>
#include <cfloat>
#include <pigpio.h>
#include <string>
#include <fstream>
#include <streambuf>

#include <list>
#include <vector>
#include <string>
#include <iostream>

#include "globals.hpp"
#include "feedback.hpp"
#include "motor.hpp"

using namespace std;

uint32_t startTick;

void completeFeedbacks();
void simProgram(); 
float timeProgram(std::list<string> commands);
void runProgram(std::list<string> commands);
void move(int distance);
void turn(int distance);

void buttonAlert(int gpio, int level, uint32_t tick, void* user);

Motor* leftMtr = NULL;
Motor* rightMtr = NULL;

int main() {
    gpioInitialise();

    gpioSetMode(BTN_GREEN, PI_INPUT);
    gpioSetPullUpDown(BTN_GREEN, PI_PUD_UP);

    gpioSetMode(BTN_BLUE, PI_INPUT);
    gpioSetPullUpDown(BTN_BLUE, PI_PUD_UP);

    gpioSetMode(MTR_ENABLE, PI_OUTPUT);
    gpioWrite(MTR_ENABLE, 0);

    gpioSetAlertFuncEx(BTN_GREEN, buttonAlert, ((void *) "Green"));
    gpioSetAlertFuncEx(BTN_BLUE, buttonAlert, ((void *) "Blue"));

    rightMtr = new Motor(RIGHT_CTL_1, RIGHT_CTL_2, RIGHT_PWM, RIGHT_SENSE_A, RIGHT_SENSE_B, false);
    rightMtr->setSpeed(0);

    leftMtr = new Motor(LEFT_CTL_1, LEFT_CTL_2, LEFT_PWM, LEFT_SENSE_A, LEFT_SENSE_B, true);
    leftMtr->setSpeed(0);

    gpioSetTimerFunc(0, 10, completeFeedbacks);

    startTick = gpioTick();

    gpioWrite(MTR_ENABLE, 1);

    move(1000);
    cout << "Done!";

    gpioWrite(MTR_ENABLE, 0);

    leftMtr->setSpeed(0);
    rightMtr->setSpeed(0);
    return 0;


    rightMtr->setSpeed(-0.35);
    leftMtr->setSpeed(-0.35);
    usleep(200000);

    Feedback* leftFeedback = new Feedback(-1000, 0.014, 0.00003, 0.000, 0.50, leftMtr->position);
    Feedback* rightFeedback = new Feedback(-1000, 0.014, 0.00003, 0.000, 0.50, rightMtr->position);

    leftFeedback->partner = rightFeedback;
    rightFeedback->partner = leftFeedback;

    leftMtr->feedback = leftFeedback;
    rightMtr->feedback = rightFeedback;

    sleep(4);

    // L90: 
    // R90: 
    // 180:
    // 360: 2650



    // leftMtr->kickstart();
    // rightMtr->kickstart();
    // rightMtr->setSpeed(0.55);
    // leftMtr->setSpeed(0.55);
    // usleep(500000);
    // rightMtr->setSpeed(0.40);
    // leftMtr->setSpeed(0.40);
    // usleep(250000);
    // rightMtr->setSpeed(0.50);
    // leftMtr->setSpeed(0.50);
    // sleep(5.5);

    gpioWrite(MTR_ENABLE, 0);

    leftMtr->setSpeed(0);
    rightMtr->setSpeed(0);

    return 0;
}

void simProgram() {
    std::list<string> commands;

    // S1 S2 S3
    // 1 2 3
    // B
    // R L U
    // X1 X2 X3

    commands.push_back("S1");
    commands.push_back("L");
    commands.push_back("1");
    commands.push_back("B");
    commands.push_back("R");
    commands.push_back("X1");

    //float programTime = timeProgram(commands);
    runProgram(commands);
}

float timeProgram(std::list<string> commands) {
    return 0;
}

void runProgram(std::list<string> commands) {
    for (string command : commands) {

        if        (command == "S1") {
            move(600);
        } else if (command == "S2") {
            move(1600);
        } else if (command == "S3") {
            move(2600);
        } else if (command == "S4") {
            move(3600);
        } else if (command == "1") {
            move(1000);
        } else if (command == "2") {
            move(2000);
        } else if (command == "3") {
            move(3000);
        } else if (command == "B") {
            move(-1000);
        } else if (command == "R") {
            turn(660);
        } else if (command == "L") {
            turn(-660);
        } else if (command == "U") {
            turn(-1320);
        } else if (command == "X1") {
            move(900);
        } else if (command == "X2") {
            move(1900);
        } else if (command == "X3") {
            move(2900);
        }
    }
}

void move(int distance) {
    rightMtr->setSpeed(0.35 * (distance < 0 ? -1 : 1));
    leftMtr->setSpeed(0.35 * (distance < 0 ? -1 : 1));
    usleep(200000);

    Feedback* leftFeedback = new Feedback(distance, 0.014, 0.00003, 0.000, 0.50, leftMtr->position);
    Feedback* rightFeedback = new Feedback(distance, 0.014, 0.00003, 0.000, 0.50, rightMtr->position);

    leftFeedback->partner = rightFeedback;
    rightFeedback->partner = leftFeedback;

    leftMtr->feedback = leftFeedback;
    rightMtr->feedback = rightFeedback;

    while (rightMtr->feedback || leftMtr->feedback) {
        usleep(10000);
    }
}

void turn(int distance) {

}


void completeFeedbacks() {
    uint32_t nowTick = gpioTick();

    // See if neither motor has moved in 10ms
    if (rightMtr->feedback && nowTick - rightMtr->feedback->prevTick > 10000 &&
        leftMtr->feedback && nowTick - leftMtr->feedback->prevTick > 10000)
    {
        rightMtr->feedback = NULL;
        rightMtr->feedback->completed = true;        
        leftMtr->feedback = NULL;
        leftMtr->feedback->completed = true;        
    }
}





    // std::vector
    // char* program = "FRF"







    // std::ifstream t("/home/darylc/robot24/route.txt");
    // std::stringstream buffer;
    // buffer << t.rdbuf();
    // cout << buffer.str;


void buttonAlert(int gpio, int level, uint32_t tick, void* user) {
    cout << (const char *) user << " button " << (level ? "released" : "pressed ") << " at " << tick << "\n";
}


/*

init all pins
motors off

read config file

green light

wait for green button

moves one at a time
calculate time ratio

*/
