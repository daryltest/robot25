#include <queue>
#ifndef MOTOR_HPP
#define MOTOR_HPP

class Motor {
public:
    int pinCtl1, pinCtl2, pinPwm, pinSenseA, pinSenseB;
    bool invert;
    int lastSensePin, lastSenseA, lastSenseB;
    int position;
    std::queue<int> recentPositions;
    std::queue<uint32_t> recentPosTicks;
    float power;
    float speed; // dPos/dTick

    Feedback* feedback;

    Motor(int pinCtl1, int pinCtl2, int pinPwm, int pinSenseA, int pinSenseB, bool invert);
    void setPower(float power);
    static void _senseAlert(int gpio, int level, uint32_t tick, void* user);
    void senseAlert(int gpio, int level, uint32_t tick);
    void updateSpeed(int position, uint32_t tick);
};

extern Motor* leftMtr;
extern Motor* rightMtr;

#endif