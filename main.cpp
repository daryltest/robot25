// g++ -Wall -pthread -o prog main.cpp -lpigpio -lrt

#include <iostream>
#include <unistd.h>
#include <math.h>
#include <cfloat>
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
#define SLOW_START_PWM_DUTY 0.45

uint32_t startTick;

void buttonAlert(int gpio, int level, uint32_t tick, void* user);

class Feedback {
public:
    uint32_t prevTick;
    int target;
    float kp, kd, ki;
    float maxPower;
    float errPrev, errInteg;
    int prevPos;
    float velocity;

    Feedback(int target, float kp, float kd, float ki, float maxPower, int prevPos);
    float update(int pos, uint32_t tick);
};

class Motor {
public:
    int pinCtl1, pinCtl2, pinPwm, pinSenseA, pinSenseB;
    bool invert;
    int lastSensePin, lastSenseA, lastSenseB;
    int position;

    Feedback* feedback;

    Motor(int pinCtl1, int pinCtl2, int pinPwm, int pinSenseA, int pinSenseB, bool invert);
    void setSpeed(float pwm);
    static void _senseAlert(int gpio, int level, uint32_t tick, void* user);
    void senseAlert(int gpio, int level, uint32_t tick);
    void kickstart();
};

Feedback::Feedback(int target, float kp, float kd, float ki, float maxPower, int prevPos):
    target(target), kp(kp), kd(kd), ki(ki), maxPower(maxPower), errPrev(0), errInteg(0), prevPos(prevPos)
{
    prevTick = gpioTick();
    velocity = 0;
}

float Feedback::update(int pos, uint32_t tick) {
    if (tick == prevTick) {
        return FLT_MAX; // Let us not divide by zero
    }

    float deltaT = (tick - prevTick) / 1e6;
    prevTick = tick;

    int deltaPos = pos - prevPos;
    velocity = deltaPos / deltaT;

    float err = target - pos;

    float dErr_dT = (err - errPrev) / deltaT;
    errPrev = err;

    errInteg += err * deltaT;
    
    float control = kp * err + kd * dErr_dT + ki * errInteg;

    if (control > maxPower) {
        control = maxPower;
    }
    else if (control < -maxPower) {
        control = -maxPower;
    }

    // Slow start
    // if ((velocity < 50000) && (control > SLOW_START_PWM_DUTY)) {
    //     control = SLOW_START_PWM_DUTY;
    // }
    // if ((velocity > -50000) && (control < -SLOW_START_PWM_DUTY)) {
    //     control = -SLOW_START_PWM_DUTY;
    // }

    return control;
}

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

    rightMtr->setSpeed(-0.35);
    leftMtr->setSpeed(+0.35);
    usleep(200000);

    Feedback* leftFeedback = new Feedback(2640, 0.015, 0.00003, 0.000, 0.50, 0);
    leftMtr->feedback = leftFeedback;

    Feedback* rightFeedback = new Feedback(-2640, 0.015, 0.00003, 0.000, 0.48, 0);
    rightMtr->feedback = rightFeedback;

    sleep(2);
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

void buttonAlert(int gpio, int level, uint32_t tick, void* user) {
    cout << (const char *) user << " button " << (level ? "released" : "pressed ") << " at " << tick << "\n";
}

Motor::Motor(int pinCtl1, int pinCtl2, int pinPwm, int pinSenseA, int pinSenseB, bool invert):
    pinCtl1(pinCtl1), pinCtl2(pinCtl2), pinPwm(pinPwm), pinSenseA(pinSenseA), pinSenseB(pinSenseB), invert(invert), position(0),
    feedback(NULL)
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
    lastSensePin = gpio;

    if (gpio == pinSenseA) {
        lastSenseA = level;
        position += ((lastSenseB == level) != invert) ? -1 : +1;
    }
    else {
        lastSenseB = level;
        position += ((lastSenseA == level) != invert) ? +1 : -1;
    }

    printf("%s", this == rightMtr ? "RIGHT sense " : "LEFT sense  ");

    printf("Lpos=%-6i  Rpos=%-6i  time=%-8u", leftMtr->position, rightMtr->position, tick - startTick);

    if (feedback != NULL) {
        float control = feedback->update(position, tick);

        if (control != FLT_MAX) {
            setSpeed(control);
            printf("  ctl=%f", control);
        }
        else {
            printf("  ctl=FLT_MAX");
        }
    }

    printf("\n");
}

void Motor::kickstart() {
    if (feedback != NULL) {
        if (feedback->target > position) {
            setSpeed(+0.25);
        }
        else {
            setSpeed(-0.25);
        }
    }
}

/*

init all pins
motors off

read config file

green light
green button

moves one at a time

*/
