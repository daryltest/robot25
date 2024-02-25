#ifndef FEEDBACK_HPP
#define FEEDBACK_HPP

class Feedback {
public:
    uint32_t prevTick;
    int target;
    float kp, kd, ki;
    float maxPower;
    float errPrev, errInteg;
    int prevPos;
    bool completed;
    Feedback* partner;

    Feedback(int target, float kp, float kd, float ki, float maxPower, int prevPos);
    float update(int pos, uint32_t tick);
};

#endif
