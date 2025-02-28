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

void Motor::setPower(float power) {
    this->power = power;
    float pwm = abs(power);

    bool fwd = (power >= 0);
    
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
            setPower(control);
            // printf("  ctl=%f", control);
        }
        else {
            // printf("  ctl=FLT_MAX");
        }
    }

    printf("  Lpower=%-4.2f  Rpower=%-4.2f", leftMtr->power, rightMtr->power);

    printf("\n");
}
