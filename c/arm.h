#ifndef ARM_H
#define ARM_H

#include <stdint.h>
#include "steering_motor.h"
#include "arm_motor.h"

//The motor value on the board
#define BASE 0xA
#define ELBOW 0xB
#define WRIST 0xC
#define CLAW 0xD

#define STEERING_GEAR_RATIO     172
#define WHEEL_GEAR_RATIO        212

#define MAX_STEERING_TICKS      300

 typedef enum {
    ARM_CALIBRATE_BASE,
    ARM_CALIBRATE_ELBOW,
    ARM_CALIBRATE_WRIST,
    ARM_CALIBRATE_CLAW,
    ARM_CALIBRATE_READY
} arms_calibrate_state_t;

// extern static struct arm_motor_t arm_motor_array[4];
// extern static struct motor_t arm_motor_subarray[4];

void arm_init();

arms_calibrate_state_t arm_calibrate();

void arm_update_steering();

void set_joints_angle(int16_t base_angle, int16_t elbow_angle, int16_t elbow_angle);

// void arm_init();

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
