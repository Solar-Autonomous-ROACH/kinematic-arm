#ifndef ROVERCORE_ROVER_H
#define ROVERCORE_ROVER_H

// #include <stdint.h>
// #include "steering_motor.h"
// #include "motor.h"
// #include "move.h"
#include "logger.h"
#include <stdint.h>
#include <bool.h>

// #define RRS                     0x0
// #define REAR_RIGHT_STEERING     0x0

// #define RRW                     0x1
// #define REAR_RIGHT_WHEEL        0x1

// #define FRS                     0x2
// #define FRONT_RIGHT_STEERING    0x2

// #define FRW                     0x3
// #define FRONT_RIGHT_WHEEL       0x3

// #define FLS                     0x4
// #define FRONT_LEFT_STEERING     0x4

// #define FLW                     0x5
// #define FRONT_LEFT_WHEEL        0x5

// #define RLS                     0x6
// #define REAR_LEFT_STEERING      0x6

// #define RLW                     0x7
// #define REAR_LEFT_WHEEL         0x7

// #define MLW                     0x8
// #define MIDDLE_LEFT_WHEEL       0x8

// #define MRW                     0x9
// #define MIDDLE_RIGHT_WHEEL      0x9

// #define STEERING_GEAR_RATIO     172
// #define WHEEL_GEAR_RATIO        212

// #define MAX_STEERING_TICKS      300

//  typedef enum {
//     ROVER_CALIBRATE_WAITING,
//     ROVER_CALIBRATE_FR,
//     ROVER_CALIBRATE_RR,
//     ROVER_CALIBRATE_FL,
//     ROVER_CALIBRATE_RL,
//     ROVER_CALIBRATE_READY
// } rover_state_t;

// static rover_state_t rover_state;


// void rover_init();

// int rover_is_calibrated();

// void rover_calibrate();

// void rover_update_steering();

// void rover_stop();

// void rover_forward(int speed);

// void rover_reverse(int speed);

// void rover_pointTurn_CW(int speed);

// void rover_pointTurn_CCW(int speed);

// void rover_steer_forward();

// void rover_steer_right(int angle);

// void rover_steer_left(int angle);

// void rover_steer_point();

bool rover_movement_done();

void tank_turn(int16_t target_angle);

void move_forward(int16_t dist);

void move_right(int16_t dist);

#endif // ROVERCORE_ROVER_H
