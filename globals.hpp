#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

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

#define BTN_GREEN 11

#define LED_RED 16
#define LED_GREEN 20
#define LED_BLUE 21

#define MAX_PWM_DUTY 550000

extern uint32_t startTick;
extern float targetProgramDuration;

#endif
