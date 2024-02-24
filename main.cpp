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
    // simProgram();
    // return 0;

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

    

    turn(-1320);
    sleep(1);

    // move(2200);
    // sleep(1);

    // turn(660);
    // sleep(1);
    // turn(660);
    // sleep(1);

    // move(6590);
    // sleep(1);

    // turn(660);
    // sleep(1);
    // turn(660);
    // sleep(1);


    // turn(-660);

    // move(300);
    // move(-600);

    cout << "Done!\n";

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


    gpioWrite(MTR_ENABLE, 0);

    leftMtr->setSpeed(0);
    rightMtr->setSpeed(0);

    return 0;
}

void simProgram() {
    std::ifstream file("/home/darylc/robot24/route.txt");
    std::string str; 
    while (std::getline(file, str))
    {
        cout << str;
    }

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
    // runProgram(commands);
}

float timeProgram(std::list<string> commands) {
    /*
    S1 1.52
    S2 3.65
    S3 5.81
    S4: 7.95
    1: 2.40
    2: 4.56
    3: 6.66
    R: 0.88
    L: 0.83
    U: 1.52
    B: 2.42
    X1: 2.16
    X2: 4.30
    X3: 6.46
    */
    return 0;
}

void runProgram(std::list<string> commands) {
    for (string command : commands) {

        if        (command == "S1") {
            move(1355);
        } else if (command == "S2") {
            move(3520);
        } else if (command == "S3") {
            move(5720);
        } else if (command == "S4") {
            move(7925);
        } else if (command == "1") {
            move(2200);
        } else if (command == "2") {
            move(4400);
        } else if (command == "3") {
            move(6590);
        } else if (command == "B") {
            move(-2185);
        } else if (command == "R") {
            turn(660);
        } else if (command == "L") {
            turn(-660);
        } else if (command == "U") {
            turn(-1320);
        } else if (command == "X1") {
            move(1945);
        } else if (command == "X2") {
            move(4145);
        } else if (command == "X3") {
            move(6340);
        }
    }
}

void executeProgramStep(int rightDistance, int leftDistance) {
    while (rightMtr->feedback || leftMtr->feedback) {
        usleep(10000);
    }

    int rightTarget = rightMtr->position + rightDistance;
    int leftTarget = leftMtr->position + leftDistance;

    rightMtr->setSpeed(0.35 * (rightDistance < 0 ? -1 : 1));
    leftMtr->setSpeed(0.39 * (leftDistance < 0 ? -1 : 1));
    usleep(200000);

    Feedback* rightFeedback = new Feedback(rightTarget, 0.014, 0.00003, 0.000, 0.50, rightMtr->position);
    Feedback* leftFeedback = new Feedback(leftTarget, 0.014, 0.00003, 0.000, 0.50, leftMtr->position);

    rightFeedback->partner = leftFeedback;
    leftFeedback->partner = rightFeedback;

    rightMtr->feedback = rightFeedback;
    leftMtr->feedback = leftFeedback;

    while (rightMtr->feedback || leftMtr->feedback) {
        usleep(10000);
    }
}

void move(int distance) {
    int s = gpioTick();
    executeProgramStep(distance, distance);
    printf("move(%i) took %f sec\n", distance, (gpioTick() - s) / 1e6);
}

void turn(int distance) {
    int s = gpioTick();
    executeProgramStep(-distance, distance);
    printf("turn(%i) took %f sec\n", distance, (gpioTick() - s) / 1e6);
}


void completeFeedbacks() {
    uint32_t nowTick = gpioTick();

    // See if neither motor has moved in 30ms
    Feedback* rFeed = rightMtr->feedback;
    Feedback* lFeed = leftMtr->feedback;

    if (rFeed && (nowTick - rFeed->prevTick > 30000) && lFeed && nowTick - lFeed->prevTick > 10000) {
        cout << "\nCOMPLETED feeds " << nowTick << " L=" << rFeed->prevTick << " R=" << rFeed->prevTick << "\n";
        rFeed->completed = true;
        lFeed->completed = true;

        rightMtr->feedback = NULL;
        leftMtr->feedback = NULL;
    }
}




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
