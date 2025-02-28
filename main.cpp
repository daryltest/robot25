// g++ -Wall -pthread -o prog *.cpp -lpigpio -lrt

#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <cfloat>
#include <pigpio.h>
#include <string>
#include <fstream>
#include <streambuf>

#include <cstring>
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
float targetDistance = 0;
float targetProgramEndTime = 0;
uint programStep = 0;

float pacingMaxPower = 0.90;

std::vector<string> programCommands;

int greenButtonCount = 0;
int blueButtonCount = 0;

void readProgram();
void completeFeedbacks();
void badConfig();
void executeProgramStep(int rightDistance, int leftDistance);
void interruptHandler(int x);

void waitForButton();
void buttonAlert(int gpio, int level, uint32_t tick, void* user);

Motor* leftMtr = NULL;
Motor* rightMtr = NULL;

int main() {
    gpioInitialise();

    signal(SIGCONT, SIG_IGN);
    signal(SIGINT, interruptHandler);

    readProgram();

    gpioSetMode(BTN_GREEN, PI_INPUT);
    gpioSetPullUpDown(BTN_GREEN, PI_PUD_UP);
    gpioNoiseFilter(BTN_GREEN, 5000, 0);

    gpioSetMode(LED_RED, PI_OUTPUT);
    gpioWrite(LED_RED, 0);

    gpioSetMode(LED_GREEN, PI_OUTPUT);
    gpioWrite(LED_GREEN, 0);

    gpioSetMode(LED_BLUE, PI_OUTPUT);
    gpioWrite(LED_BLUE, 0);

    gpioSetMode(MTR_ENABLE, PI_OUTPUT);
    gpioWrite(MTR_ENABLE, 0);

    gpioSetAlertFuncEx(BTN_GREEN, buttonAlert, ((void *) "Green"));

    rightMtr = new Motor(RIGHT_CTL_1, RIGHT_CTL_2, RIGHT_PWM, RIGHT_SENSE_B, RIGHT_SENSE_A, false);
    rightMtr->setSpeed(0);

    leftMtr = new Motor(LEFT_CTL_1, LEFT_CTL_2, LEFT_PWM, LEFT_SENSE_B, LEFT_SENSE_A, true);
    leftMtr->setSpeed(0);

    gpioSetTimerFunc(0, 10, completeFeedbacks);

    startTick = gpioTick();

    while (true) {
        gpioWrite(LED_BLUE, 1);

        waitForButton();

        gpioWrite(LED_BLUE, 0);

        gpioWrite(MTR_ENABLE, 1);

        executeProgramStep(800, 800);

        gpioWrite(MTR_ENABLE, 0);

        leftMtr->setSpeed(0);
        rightMtr->setSpeed(0);
    }

    rightMtr->setSpeed(0);
    leftMtr->setSpeed(0);

    gpioWrite(LED_RED, 0);
    gpioWrite(LED_GREEN, 0);
    gpioWrite(LED_BLUE, 0);    
}

void interruptHandler(int x) {
    cout << "INTERRUPT\n";

    gpioWrite(MTR_ENABLE, 0);
    gpioWrite(RIGHT_PWM, 0);
    gpioWrite(LEFT_PWM, 0);
    gpioWrite(LED_RED, 0);
    gpioWrite(LED_GREEN, 0);
    gpioWrite(LED_BLUE, 0);

    exit(0);
}

void executeProgramStep(int rightDistance, int leftDistance) {
    while (rightMtr->feedback || leftMtr->feedback) {
        usleep(10000);
    }

    int rightTarget = rightMtr->position + rightDistance;
    int leftTarget = leftMtr->position + leftDistance;

    rightMtr->setSpeed(0.60 * (rightDistance < 0 ? -1 : 1));
    leftMtr->setSpeed(0.60 * (leftDistance < 0 ? -1 : 1));

    if (abs(rightDistance) < 50 && abs(leftDistance) < 50) {
        usleep(50000);
    } else {
        usleep(100000);
    }

    Feedback* rightFeedback = new Feedback(rightTarget, 0.02, 0.03, 0.00, pacingMaxPower, rightMtr->position);
    Feedback* leftFeedback = new Feedback(leftTarget, 0.02, 0.03, 0.00, pacingMaxPower, leftMtr->position);

    rightFeedback->partner = leftFeedback;
    leftFeedback->partner = rightFeedback;

    rightMtr->feedback = rightFeedback;
    leftMtr->feedback = leftFeedback;

    while (rightMtr->feedback || leftMtr->feedback) {
        usleep(10000);
    }
}

void completeFeedbacks() {
    uint32_t nowTick = gpioTick();

    // See if neither motor has moved in 30ms
    Feedback* rFeed = rightMtr->feedback;
    Feedback* lFeed = leftMtr->feedback;

    if (rFeed && (nowTick - rFeed->prevTick > 30000) && lFeed && nowTick - lFeed->prevTick > 30000) {
        cout << "\nCOMPLETED feeds " << nowTick << " L=" << rFeed->prevTick << " R=" << rFeed->prevTick << "\n";
        rFeed->completed = true;
        lFeed->completed = true;

        rightMtr->feedback = NULL;
        leftMtr->feedback = NULL;
    }
}

void waitForButton() {
    int count = greenButtonCount + blueButtonCount;

    while (count == greenButtonCount + blueButtonCount) {
        usleep(50000);
    }
}

void buttonAlert(int gpio, int level, uint32_t tick, void* user) {
    if (level && !strcmp((const char *) user, "Green")) {
        ++greenButtonCount;
    }
    if (level && !strcmp((const char *) user, "Blue")) {
        ++blueButtonCount;
    }
    
    cout << (const char *) user << " button " << (!level ? "released" : "pressed ") << " at " << tick << "\n";
}

void readProgram() {
    std::ifstream file("/boot/robotdistance.txt");
    std::string str;

    while (std::getline(file, str))
    {
        while (str[str.size() - 1] == '\r') {
            str.erase(str.size() - 1, 1);
        }

        if (str.size() == 0) {
            continue;
        }

        if (targetDistance == 0) {
            try {
                targetDistance = std::stof(str);
            }
            catch (std::invalid_argument const& ex) {
                badConfig();
            }

            continue;
        }
    }

    if (targetDistance == 0) {
        badConfig();
    }
    
    cout << "Target distance: " << targetDistance << "m\n";
}

void badConfig() {
    gpioSetMode(LED_RED, PI_OUTPUT);

    while (true) {
        gpioWrite(LED_RED, 1);
        usleep(500000);
        gpioWrite(LED_RED, 0);
        usleep(500000);
    }
}