// g++ -Wall -pthread -o prog *.cpp -lpigpio -lrt

#include <iostream>
#include <unistd.h>
#include <math.h>
#include <cfloat>
#include <pigpio.h>
#include <string>
#include <fstream>
#include <streambuf>

#include "globals.hpp"
#include "feedback.hpp"
#include "motor.hpp"

using namespace std;

uint32_t startTick;

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

    leftMtr = new Motor(LEFT_CTL_1, LEFT_CTL_2, LEFT_PWM, LEFT_SENSE_A, LEFT_SENSE_B, true);
    leftMtr->setSpeed(0);

    rightMtr = new Motor(RIGHT_CTL_1, RIGHT_CTL_2, RIGHT_PWM, RIGHT_SENSE_A, RIGHT_SENSE_B, false);
    rightMtr->setSpeed(0);

    startTick = gpioTick();

    gpioWrite(MTR_ENABLE, 1);

    rightMtr->setSpeed(+0.35);
    leftMtr->setSpeed(+0.35);
    usleep(200000);

    Feedback* leftFeedback = new Feedback(1000, 0.015, 0.00003, 0.000, 0.50, leftMtr->position);
    Feedback* rightFeedback = new Feedback(1000, 0.015, 0.00003, 0.000, 0.50, rightMtr->position);

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

void runProgram() {
    // std::ifstream t("/home/darylc/robot24/route.txt");
    // std::stringstream buffer;
    // buffer << t.rdbuf();
    // cout << buffer.str;
}


void buttonAlert(int gpio, int level, uint32_t tick, void* user) {
    cout << (const char *) user << " button " << (level ? "released" : "pressed ") << " at " << tick << "\n";
}


/*

init all pins
motors off

read config file

green light
green button

moves one at a time

*/
