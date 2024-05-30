#include "simba_bridge.h"

/* This file is basically a Armada (arm team) addon to the Simba (rover team) rover.c  
 * Basically could be combined with that file, but since this was made as a senior project addon,
 * I decided to make it a seperate file here so the entire branch for everything stays within on repo.
 **/

#include "rover.h"
#include "mmio.h"
#include "servo.h"
#include "steering_motor.h"
#include "pid.h"

#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

extern Servo servos[];


/* Rotates rover to angle, with postive x being counterclockwise
 **/

int armada_rover_rotate(int angle, double speed) {
    //Angle can be postive or negative
    long inner_tics = deg_len_ticks(angle, INNER_RADIUS);
    long outer_tics = deg_len_ticks(angle, OUTER_RADIUS);
    // printf("INNER: %ld OUTER: %ld\n", inner_tics, outer_tics);
    printf("Rotate Pointer: %p", servos);
    for (int i = 0; i < NUM_MOTORS_2; i++) {
        switch (servos[i].motor.addr) {
            //Long Distance
        case MOTOR_REAR_RIGHT_WHEEL:
        case MOTOR_FRONT_RIGHT_WHEEL:
        case MOTOR_REAR_LEFT_WHEEL:
        case MOTOR_FRONT_LEFT_WHEEL:
            //Max speed as is
            servos[i].pid.outputLimitMin = -speed;
            servos[i].pid.outputLimitMax = speed;
            //Turn postive angle should be turn to the right
            servos[i].setpoint = servos[i].counts + outer_tics;
            break;
            //Short Distance
        case MOTOR_MIDDLE_RIGHT_WHEEL:
        case MOTOR_MIDDLE_LEFT_WHEEL:
            //Shorter radius, so slower speed by ratio of how far out
            servos[i].pid.outputLimitMin = (-speed * INNER_RADIUS) / OUTER_RADIUS;
            servos[i].pid.outputLimitMax = (speed * INNER_RADIUS) / OUTER_RADIUS;
            //Same as above, but with the shorter distance
            servos[i].setpoint = servos[i].counts + inner_tics;
            break;


            //inverted
            // // set max speed
            // servos[i].pid.outputLimitMin = -speed;
            // servos[i].pid.outputLimitMax = speed;
            // // set the distance
            // servos[i].setpoint = servos[i].counts + dist_tics;
            // break;
            //regular
            // // set max speed
            // servos[i].pid.outputLimitMin = -speed;
            // servos[i].pid.outputLimitMax = speed;
            // // set the distance
            // servos[i].setpoint = servos[i].counts - dist_tics;
            // break;
        default:
            break;
        }
    }
  return 0;
}


long deg_len_ticks(int angle, int radius) {
    long arc_distance = 2 * radius * M_PI * angle / 360;
    return (arc_distance * 12) / (124 * M_PI / 116);
    //Values copied from other file as didn't want to redefine
// /* For Motion*/
// #define WHEEL_DIAMETER 124     /* mm diameter of the wheel*/
// /*this is for the 6 wheel motors*/
// #define TICKS_PER_REV_6 12                /*ticks per revolution*/
// #define GEAR_RATIO_6 116
}