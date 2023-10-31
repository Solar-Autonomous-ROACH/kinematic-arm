#ifndef ARM_MOTOR_H_
#define ARM_MOTOR_H_
#include "motor.h"
#include <stdbool.h>
#include <stdint.h>

#define CALIBRATION_SPEED 15
#define MOTOR_TICKS_ERROR_MARGIN 5
#define ACCELERATION_TIME 5000 // time motor takes to accelerate
#define MAX_SPEED 70           // motor max_speed
#define ARM_MOTOR_KP 0.02
#define CALIBRATE_MOVE_HOLD_DURATION 500 // 500ms

typedef enum {
  ARM_MOTOR_CALIBRATE_INIT,
  ARM_MOTOR_CALIBRATION_HOLD_POS_SPEED,
  ARM_MOTOR_CALIBRATION_HOLD_NEG_SPEED,
  ARM_MOTOR_CALIBRATE_POS_SPEED,
  ARM_MOTOR_CALIBRATE_NEG_SPEED,
  ARM_MOTOR_CALIBRATE_SUCCESS,
  ARM_MOTOR_CHECK_POSITION,
  ARM_MOTOR_MOVING_TO_TARGET,
} arm_motor_state_t;

typedef struct {
  uint8_t index;
  long stopper_pos; // encoder reading of stopper is stored here
  bool pos_angle; // if true the target encoder value should be < stopper position
  uint16_t move_bits; // each bit represents if it moved or not the last isr
  arm_motor_state_t state; // current state of the motor
  motor_t *motor;
  bool is_calibrated;
  long moving_time_ms; // time since the arm went from ARM_CHECK_POSITION to
                       // ARM_MOVING_TO_TARGET
  double gear_ratio;
  uint16_t CPR;
} arm_motor_t;

arm_motor_state_t calibrate_handle_state(arm_motor_t *a_motor);
arm_motor_state_t arm_motor_handle_state(arm_motor_t *motor);
bool check_stopped(arm_motor_t *s_motor);

#endif
