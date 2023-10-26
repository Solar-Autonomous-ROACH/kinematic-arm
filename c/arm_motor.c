#include "arm_motor.h"

#define buf_size 20

long pos[buf_size];
long pos_index = 0;
long spinup = 20;
long vel = 0;

/**
 * @brief Control a single a_motor. only works if a_motor is calibrated
 * @author Christian Honein
 * @param a_motor
 * @return arm_motor_state_t the state the arm is heading to
 */
arm_motor_state_t arm_motor_handle_state(arm_motor_t *a_motor) {
  if (!a_motor->is_calibrated) {
    // don't do anything if arm is not calibrated
    return a_motor->state;
  }

  long current_position = a_motor->motor->abs_pos;
  long target_position = a_motor->motor->target_pos;
  long diff = target_position - current_position;
  long abs_diff = diff < 0 ? -diff : diff;
  long speed_reducer = 0;
  long abs_speed = 0;
  // long diff = a_motor.

  motor_update(a_motor->index);
  switch (a_motor->state) {

  case ARM_MOTOR_CHECK_POSITION:
    if (diff > MOTOR_TICKS_ERROR_MARGIN) {
      a_motor->state = ARM_MOTOR_MOVING_TO_TARGET;
      a_motor->moving_time_ms = 0;
    } else {
      set_motor_speed(a_motor->index, 0);
    }
    break;

  case ARM_MOTOR_MOVING_TO_TARGET:
    a_motor->moving_time_ms++;
    if (abs_diff > MOTOR_TICKS_ERROR_MARGIN) {
      abs_speed = abs_diff * ARM_MOTOR_KP;
      if (a_motor->moving_time_ms < ACCELERATION_TIME) {
        speed_reducer = (MAX_SPEED - 10) *
                        (ACCELERATION_TIME - a_motor->moving_time_ms) /
                        ACCELERATION_TIME;
      }
      if (abs_speed > MAX_SPEED) {
        abs_speed = MAX_SPEED;
      }
      abs_speed = abs_speed - speed_reducer;
      if (abs_speed < 10) {
        abs_speed = 10;
      }
      if (diff > 0) {
        set_motor_speed(a_motor->index, abs_speed);
      } else {
        set_motor_speed(a_motor->index, -abs_speed);
      }
    } else {
      a_motor->state = ARM_MOTOR_CHECK_POSITION;
      set_motor_speed(a_motor->index, 0);
    }
    break;

  default:
    break;
  }
  return a_motor->state;
}

/**
 * @brief state machine to calibrate a single a_motor
 * @author Wesley Luu
 * @param a_motor
 * @return int
 */
arm_motor_state_t calibrate_handle_state(arm_motor_t *a_motor) {
  // motor_update(a_motor->index);
  switch (a_motor->state) {
  case ARM_MOTOR_CALIBRATE_INIT:
    // initialize some values
    a_motor->motor = get_motor(a_motor->index);
    a_motor->high_pos = 0;
    a_motor->low_pos = 0;
    a_motor->move_bits = 0;
    a_motor->is_calibrated = false;

    // next state
    a_motor->state = ARM_MOTOR_CALIBRATION_HOLD_POS_SPEED;
    set_motor_speed(a_motor->index, CALIBRATION_SPEED);
    a_motor->moving_time_ms = 0;
    break;
  
  case ARM_MOTOR_CALIBRATION_HOLD_POS_SPEED:
    if (a_motor->moving_time_ms > CALIBRATE_MOVE_HOLD_DURATION) {
      a_motor->state = ARM_MOTOR_CALIBRATE_POS_SPEED;
    }
    break;

  case ARM_MOTOR_CALIBRATE_POS_SPEED:
    if (check_stopped(a_motor)) { // if motor has stopped
        // now have high position
        a_motor->high_pos = a_motor->motor->abs_pos;
        // next state
        a_motor->moving_time_ms = 0;
        a_motor->state = ARM_MOTOR_CALIBRATION_HOLD_POS_SPEED;
        set_motor_speed(a_motor->index, -CALIBRATION_SPEED);
    }
    break;
  
  case ARM_MOTOR_CALIBRATION_HOLD_NEG_SPEED:
    if (a_motor->moving_time_ms > CALIBRATE_MOVE_HOLD_DURATION) {
      a_motor->state = ARM_MOTOR_CALIBRATE_NEG_SPEED;
    }
    break;

  case ARM_MOTOR_CALIBRATE_NEG_SPEED:
    if (check_stopped(a_motor)) { // if motor has stopped
        // now have low position
        a_motor->low_pos = a_motor->motor->abs_pos;
        // next state
        a_motor->state = ARM_MOTOR_CALIBRATE_SUCCESS;
        a_motor->is_calibrated = true;
        set_motor_speed(a_motor->index, 0);
    }
    break;

  default:
    return -1;
    break;
  }

  a_motor->moving_time_ms++;
  return  a_motor->state;
}

/**
 * @brief Function to update if movement was recorded this call and return if
 * true if has not moved in the last 16 calls
 * @author Luke Cartier
 * @param a_motor
 * @return int
 */
int check_stopped(arm_motor_t *s_motor) {
  int cur_speed;
  uint16_t movement_bits;
  cur_speed = get_motor_velocity(s_motor->index);
  movement_bits = s_motor->move_bits << 1; // preps value to add
  s_motor->move_bits = movement_bits | (cur_speed && 0x0001); // updates value
  if (s_motor->move_bits) { // If all 0's, is false, so hasn't moved for 16 isr
                            // cycles
    return true;
  } else { // has moved recently
    return false;
  }
}