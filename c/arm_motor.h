#include <stdint.h>
#include "motor.h"

#ifndef C_ARM_MOTOR_H
#define C_ARM_MOTOR_H

#define CALIBRATION_SPEED 30

 typedef enum {
    STATE_INITIALIZE,
    STATE_WAITING,
    STATE_CALIBRATION_LEFT,
    STATE_CALIBRATION_RIGHT,
    STATE_CALIBRATION_CENTER,
    STATE_READY
} arm_state_t;

typedef struct {
    uint8_t index;
    long left_pos;
    long right_pos;
    long center_pos;
    int target;
    arm_state_t state;  // current state of the motor
} arm_motor_t;


void calibrate (arm_motor_t *s_motor);
int arm_motor_handle_state(arm_motor_t *s_motor);

#endif //C_STEERING_MOTOR_H
