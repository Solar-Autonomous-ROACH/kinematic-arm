#ifndef ARM_MOTOR_H_
#define ARM_MOTOR_H_
#include "arm_motor_controller.h"
#include <stdbool.h>
#include <stdint.h>

#define MOTOR_TICKS_ERROR_MARGIN 3
#define MAX_SPEED 65 // motor max_speed
// time (ms) to hold speed before checking for stop
#define CALIBRATE_MOVE_HOLD_DURATION 1000
// time (ms) motor has to be stopped to be at a stop
#define CALIBRATION_STOP_DURATION 16

// time (ms) to wait after motor starts moving before checking for stalls
#define MOTOR_STALL_START_CHECKING_TIME_MS 500
// time (ms) motor has to be stopped to determine a stall
#define MOTOR_STALL_DURATION 100

typedef enum {
  ARM_MOTOR_CALIBRATE_INIT,
  ARM_MOTOR_CALIBRATION_HOLD_POS_SPEED,
  ARM_MOTOR_CALIBRATION_HOLD_NEG_SPEED,
  ARM_MOTOR_CALIBRATE_POS_SPEED,
  ARM_MOTOR_CALIBRATE_NEG_SPEED,
  ARM_MOTOR_CALIBRATE_IN_PROGRESS,
  ARM_MOTOR_CHECK_POSITION,
  ARM_MOTOR_MOVING_TO_TARGET,
  ARM_MOTOR_ERROR
} arm_motor_state_t;

typedef struct {
  char *name;
  uint8_t index;
  long stopper_pos; // encoder reading of stopper is stored here
  bool pos_angle;   // if true the target encoder value should be < stopper
                    // position
  arm_motor_state_t state; // current state of the motor
  motor_t *motor;
  bool is_calibrated;
  unsigned long moving_time_ms; // time since the arm went from
                                // ARM_CHECK_POSITION to ARM_MOVING_TO_TARGET
  double gear_ratio;
  uint16_t CPR;
  int8_t calibration_speed;
  int8_t min_speed;
  // values for PID controller
  double kp;
  double ki;
  double kd;
  double integral;
  long integral_threshold;
} arm_motor_t;

arm_motor_state_t calibrate_handle_state(arm_motor_t *a_motor);
arm_motor_state_t arm_motor_handle_state(arm_motor_t *motor);
bool check_stopped(arm_motor_t *a_motor, uint16_t duration);
double get_motor_angle(arm_motor_t *a_motor);

#endif
