#ifndef CLAW_H
#define CLAW_H

#include "arm.h"
#include <stdbool.h>
#include <stdint.h>

#define CLAW_ERROR_MARGIN 1
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

void claw_init();
claw_state_t claw_handle_state();
void set_claw_angle(uint16_t angle);
void open_claw();
void close_claw();

#endif