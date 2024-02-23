// g++ -Wall -pthread -o prog main.cpp -lpigpio -lrt

#include <iostream>
#include <unistd.h>
#include <pigpio.h>

using namespace std;

#define MTR_ENABLE 25

#define LEFT_PWM 12
#define LEFT_CTL_1 10
#define LEFT_CTL_2 24
#define LEFT_SENSE_A 23
#define LEFT_SENSE_B 22

#define RIGHT_PWM 13
#define RIGHT_CTL_1 4
#define RIGHT_CTL_2 27
#define RIGHT_SENSE_A 17
#define RIGHT_SENSE_B 18

#define BTN_GREEN 9
#define BTN_BLUE 11

#define LED_RED 16
#define LED_GREEN 20
#define LED_BLUE 21

#define MAX_PWM_DUTY 550000

int totalSense = 0, badSense = 0;

void buttonAlert(int gpio, int level, uint32_t tick, void* user);

class Motor {
public:
    int pinCtl1, pinCtl2, pinPwm, pinSenseA, pinSenseB;
    bool invert;
    int lastSensePin, lastSenseA, lastSenseB;
    int position;

    Motor(int pinCtl1, int pinCtl2, int pinPwm, int pinSenseA, int pinSenseB, bool invert);
    void setSpeed(float pwm);
    static void _senseAlert(int gpio, int level, uint32_t tick, void* user);
    void senseAlert(int gpio, int level, uint32_t tick);
};

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

    gpioWrite(MTR_ENABLE, 1);

    rightMtr->setSpeed(0.25);
    leftMtr->setSpeed(0.25);
    usleep(250000);
    rightMtr->setSpeed(0.40);
    leftMtr->setSpeed(0.40);
    usleep(250000);
    rightMtr->setSpeed(0.50);
    leftMtr->setSpeed(0.50);
    sleep(3);

    gpioWrite(MTR_ENABLE, 0);

    cout << "Hello World!\n";
    cout << badSense << " sense edges dropped out of " << totalSense << "\n";
    return 0;
}

void buttonAlert(int gpio, int level, uint32_t tick, void* user) {
    cout << (const char *) user << " button " << (level ? "released" : "pressed ") << " at " << tick << "\n";
}

Motor::Motor(int pinCtl1, int pinCtl2, int pinPwm, int pinSenseA, int pinSenseB, bool invert):
    pinCtl1(pinCtl1), pinCtl2(pinCtl2), pinPwm(pinPwm), pinSenseA(pinSenseA), pinSenseB(pinSenseB), invert(invert), position(0)
{
    gpioSetMode(pinCtl1, PI_OUTPUT);
    gpioSetMode(pinCtl2, PI_OUTPUT);
    gpioSetMode(pinPwm, PI_OUTPUT);
    gpioSetMode(pinSenseA, PI_INPUT);
    gpioSetMode(pinSenseB, PI_INPUT);

    gpioSetAlertFuncEx(pinSenseA, _senseAlert, this);
    gpioSetAlertFuncEx(pinSenseB, _senseAlert, this);
}

void Motor::setSpeed(float pwm) {
    bool fwd = true;
    if (pwm < 0) {
        fwd = false;
        pwm = -pwm;
    }

    if (pwm == 0) {
        // Stop
        gpioWrite(pinCtl1, 0);
        gpioWrite(pinCtl2, 0);
        gpioWrite(pinPwm, 0);
    }
    else if (fwd != invert) {
        gpioWrite(pinCtl1, 1);
        gpioWrite(pinCtl2, 0);
        gpioHardwarePWM(pinPwm, 80000, (int) (pwm * MAX_PWM_DUTY));
    }
    else {
        gpioWrite(pinCtl1, 0);
        gpioWrite(pinCtl2, 1);
        gpioHardwarePWM(pinPwm, 80000, (int) (pwm * MAX_PWM_DUTY));
    }
}

void Motor::_senseAlert(int gpio, int level, uint32_t tick, void* user) {
    ((Motor*) user)->senseAlert(gpio, level, tick);
}

void Motor::senseAlert(int gpio, int level, uint32_t tick) {
    ++totalSense;
    if (gpio == lastSensePin) {
        // This shouldn't happen. We got 2 A's in a row without a B, or vice-versa.
        ++badSense;
        return;
    }
    lastSensePin = gpio;

    if (gpio == pinSenseA) {
        lastSenseA = level;
        position += ((lastSenseB == level) != invert) ? -1 : +1;
    }
    else {
        lastSenseB = level;
        position += ((lastSenseA == level) != invert) ? +1 : -1;
    }

    printf("L %8i R %8i t %-8i\n", leftMtr->position, rightMtr->position, tick);
}


/*

init all pins
motors off

read config file

green light
green button

moves one at a time

*/