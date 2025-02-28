#ifndef MOTOR_HPP
#define MOTOR_HPP

class Motor {
public:
    int pinCtl1, pinCtl2, pinPwm, pinSenseA, pinSenseB;
    bool invert;
    int lastSensePin, lastSenseA, lastSenseB;
    int position;
    float speed;

    Feedback* feedback;

    Motor(int pinCtl1, int pinCtl2, int pinPwm, int pinSenseA, int pinSenseB, bool invert);
    void setSpeed(float pwm);
    static void _senseAlert(int gpio, int level, uint32_t tick, void* user);
    void senseAlert(int gpio, int level, uint32_t tick);
    void kickstart();
};

extern Motor* leftMtr;
extern Motor* rightMtr;

#endif