#ifndef ARM_H
#define ARM_H

#include <stdint.h>
#include "steering_motor.h"
#include "motor.h"
#include "move.h"

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

void rover_init();

int rover_is_calibrated();

void rover_calibrate();

void rover_update_steering();

void rover_stop();

void rover_forward(int speed);

void rover_reverse(int speed);

void rover_pointTurn_CW(int speed);

void rover_pointTurn_CCW(int speed);

void rover_steer_forward();

void rover_steer_right(int angle);

void rover_steer_left(int angle);

void rover_steer_point();





#endif //ROVERCORE_ROVER_H
