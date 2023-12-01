#ifndef CLAW_H
#define CLAW_H

#include <stdint.h>
#include <stdbool.h>
#include "arm.h"

#define CLAW_ERROR_MARGIN 5
#define CLAW_SPEED 30

typedef enum {
  CLAW_CHECK_POSITION,
  CLAW_ROTATING,
  CLAW_OPENING_CLOSING
} claw_state_t;

typedef struct {
    claw_state_t state;
    uint8_t index;
    motor_t *motor;
    bool is_open;
    double gear_ratio;
    uint16_t CPR;
    long current_angle_ticks;
    long target_angle_ticks;
    bool target_is_open;
} claw_motor_t;

claw_state_t claw_handle_state(claw_motor_t * c_motor);
void set_claw_angle(claw_motor_t * c_motor, uint16_t angle);
void open_claw(claw_motor_t *c_motor);
void close_claw(claw_motor_t *c_motor);
bool claw_rotation_complete(claw_motor_t * c_motor);


#endif