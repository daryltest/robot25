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

Feedback::Feedback(int target, float kp, float kd, float ki, float maxPower, int prevPos):
    target(target), kp(kp), kd(kd), ki(ki), maxPower(maxPower), errPrev(0), errInteg(0), prevPos(prevPos), completed(false)
{
    prevTick = gpioTick();
}

float Feedback::update(int pos, uint32_t tick) {
    if (tick == prevTick) {
        return FLT_MAX; // Let us not divide by zero
    }

    if (completed) {
        return 0;
    }

    float deltaT = (tick - prevTick) / 1e6;
    prevTick = tick;

    float err = target - pos;
    prevPos = pos;

    float dErr_dT = (err - errPrev) / deltaT;
    errPrev = err;

    errInteg += err * deltaT;
    
    float control = kp * err + kd * dErr_dT + ki * errInteg;

    float maxPower = this->maxPower;
    
    if (partner != NULL) {
        int partnerAhead = abs(this->target - this->prevPos) - abs(partner->target - partner->prevPos);

        if (partnerAhead >= 3) {
            maxPower += 0.03;
        }
    }

    if (control > maxPower) {
        control = maxPower;
    }
    else if (control < -maxPower) {
        control = -maxPower;
    }

    return control;
}
