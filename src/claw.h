#ifndef CLAW_H
#define CLAW_H

#include "arm_motor_api.h"
#include "rover.h"
#include <stdbool.h>
#include <stdint.h>

#define CLAW_ERROR_MARGIN 1
#define CLAW_SPEED 30
#define CLAW_CALIBRATION_PERIOD 2

// #define HALL_EFFECT_ADDRESS 0x80110000
#define HALL_EFFECT_ADDRESS NULL

typedef enum {
  CLAW_CALIBRATE_START,
  CLAW_CALIBRATE_WAIT_FOR_MAGNET,
  CLAW_CALIBRATE_WAIT_FOR_TURN,
  CLAW_CALIBRATE_VERIFY_OPEN_CLOSE_MAGNET,
  CLAW_CALIBRATE_GOTO_OPEN_CLOSE_MAGNET,
  CLAW_CALIBRATE_OPEN_CLAW,
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
void set_claw_speed(uint8_t speed, bool openclose);
void claw_goto_calibrate();
void set_claw_angle(uint16_t angle);
void open_claw();
void close_claw();
void toggle_claw();

#endif