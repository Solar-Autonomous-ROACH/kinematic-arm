#ifndef ARM_H
#define ARM_H

#include <stdint.h>
#include "steering_motor.h"
#include "motor.h"

//The motor value on the board
#define BASE 0xA
#define ELBOW 0xB
#define WRIST 0xC
#define CLAW 0xD

#define STEERING_GEAR_RATIO     172
#define WHEEL_GEAR_RATIO        212

#define MAX_STEERING_TICKS      300

 typedef enum {
    ARM_CALIBRATE_WAITING,
    ARM_CALIBRATE_FR,
    ARM_CALIBRATE_RR,
    ARM_CALIBRATE_FL,
    ARM_CALIBRATE_RL,
    ARM_CALIBRATE_READY
} arm_state_t;

static arm_state_t arm_state;

static steering_motor_t steer_BASE;
static steering_motor_t steer_ELBOW;
static steering_motor_t steer_WRIST;
static steering_motor_t steer_CLAW;

void arm_init();

int arm_is_calibrated();

void arm_calibrate();

void arm_update_steering();

// void rover_stop();

// void arm_forward(int speed);

// void arm_reverse(int speed);

// void arm_pointTurn_CW(int speed);

// void arm_pointTurn_CCW(int speed);

// void arm_steer_forward();

// void arm_steer_right(int angle);

// void arm_steer_left(int angle);

// void arm_steer_point();





#endif //ROVERCORE_ROVER_H
