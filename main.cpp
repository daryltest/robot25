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

float targetProgramDuration = 0;

void completeFeedbacks();
void simProgram(); 
float timeProgram(std::vector<string> commands);
void runProgram(std::vector<string> commands);
void runCommand(string command);
void move(int distance);
void turn(int distance);
float nominalProgramTimeRemaining(std::vector<string> commands);
float nominalCommandTime(string command);

void buttonAlert(int gpio, int level, uint32_t tick, void* user);

Motor* leftMtr = NULL;
Motor* rightMtr = NULL;

int main() {
    // read program, build vector; calculate estimated and target durations

    gpioInitialise();

    gpioSetMode(BTN_GREEN, PI_INPUT);
    gpioSetPullUpDown(BTN_GREEN, PI_PUD_UP);

    gpioSetMode(BTN_BLUE, PI_INPUT);
    gpioSetPullUpDown(BTN_BLUE, PI_PUD_UP);

    gpioSetMode(LED_RED, PI_OUTPUT);
    gpioWrite(LED_RED, 0);

    gpioSetMode(LED_GREEN, PI_OUTPUT);
    gpioWrite(LED_GREEN, 0);

    gpioSetMode(LED_BLUE, PI_OUTPUT);
    gpioWrite(LED_BLUE, 0);

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

    while (true) {
        gpioWrite(LED_BLUE, 1);

        // sleep till green button

        gpioWrite(LED_BLUE, 0);
        gpioWrite(LED_GREEN, 1);

        gpioWrite(MTR_ENABLE, 1);

        // motor around

        gpioWrite(MTR_ENABLE, 0);
        leftMtr->setSpeed(0);
        rightMtr->setSpeed(0);

        gpioWrite(LED_GREEN, 0);
        gpioWrite(LED_RED, 1);
        // sleep till blue button

    }
}

void readProgram() {
    std::ifstream file("/home/darylc/robot24/route.txt");
    std::string str;

    while (std::getline(file, str))
    {


        cout << str << "\n";
    }

    targetProgramDuration = 0;
}

void runProgram(std::vector<string> commands) {
    for (string command : commands) {
        runCommand(command);
    }
}

void runCommand(string command) {
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

float nominalProgramTimeRemaining(std::vector<string> commands, int startStep) {
    float remaining = 0;

    for (uint i = startStep; i < commands.size(); ++i) {
        remaining += nominalCommandTime(commands[i]);
    }

    return remaining;
}

float nominalCommandTime(string command) {
    if        (command == "S1") {
        return 1.52;
    } else if (command == "S2") {
        return 3.65;
    } else if (command == "S3") {
        return 5.81;
    } else if (command == "S4") {
        return 7.95;
    } else if (command == "1") {
        return 2.40;
    } else if (command == "2") {
        return 4.56;
    } else if (command == "3") {
        return 6.66;
    } else if (command == "B") {
        return 2.42;
    } else if (command == "R") {
        return 0.88;
    } else if (command == "L") {
        return 0.83;
    } else if (command == "U") {
        return 1.52;
    } else if (command == "X1") {
        return 2.16;
    } else if (command == "X2") {
        return 4.30;
    } else if (command == "X3") {
        return 6.46;
    }

    return 0;
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
    // executeProgramStep(distance, distance);
    printf("move(%i) took %f sec\n", distance, (gpioTick() - s) / 1e6);
}

void turn(int distance) {
    int s = gpioTick();
    // executeProgramStep(-distance, distance);
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
