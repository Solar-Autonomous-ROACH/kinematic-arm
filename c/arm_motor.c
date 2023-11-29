#include "arm_motor.h"
#include "logger.h"

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
  long abs_speed = 0;
  long abs_diff_velocity =
      get_motor_position_n(a_motor->index, 0) -
      get_motor_position_n(a_motor->index, POSITION_FIFO_SIZE - 1);
  abs_diff_velocity =
      abs_diff_velocity >= 0 ? abs_diff_velocity : -abs_diff_velocity;

  switch (a_motor->state) {
  case ARM_MOTOR_CHECK_POSITION:
    if (abs_diff > MOTOR_TICKS_ERROR_MARGIN) {
      log_message(LOG_DEBUG, "%s: Current: %ld, Target: %ld\n", a_motor->name,
                  current_position, target_position);
      a_motor->state = ARM_MOTOR_MOVING_TO_TARGET;
      a_motor->moving_time_ms = 0;
      a_motor->integral = 0;
    } else {
      set_motor_speed(a_motor->index, 0);
    }
    break;

  case ARM_MOTOR_MOVING_TO_TARGET:
    a_motor->moving_time_ms++;
    if (abs_diff > MOTOR_TICKS_ERROR_MARGIN) {
      abs_speed = a_motor->kp * abs_diff;
      if (abs_diff < a_motor->integral_threshold) {
        a_motor->integral += abs_diff * a_motor->ki;
        abs_speed += a_motor->integral;
      }
      if (abs_speed > MAX_SPEED) {
        abs_speed = MAX_SPEED;
      }
      abs_speed -= a_motor->kd * abs_diff_velocity;

      if (diff > 0) {
        set_motor_speed(a_motor->index, abs_speed);
      } else {
        set_motor_speed(a_motor->index, -abs_speed);
      }
    } else {
      a_motor->state = ARM_MOTOR_CHECK_POSITION;
      log_message(LOG_INFO, "%s reached position\n", a_motor->name);
      set_motor_speed(a_motor->index, 0);
    }
    if (a_motor->moving_time_ms % 100 == 0) {
      log_message(
          LOG_DEBUG,
          "Current: %ld, Target: %ld, speed: %ld, abs_diff_velocity: %ld, "
          "integral: %f\n",
          current_position, target_position, abs_speed, abs_diff_velocity,
          a_motor->integral);
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
    a_motor->stopper_pos = 0;
    a_motor->motor->stopped_duration = 0;
    a_motor->is_calibrated = false;

    if (!(a_motor->pos_angle)) {
      // we have to go counter clock wise to get to stopper
      a_motor->state = ARM_MOTOR_CALIBRATION_HOLD_POS_SPEED;
      set_motor_speed(a_motor->index, a_motor->calibration_speed);
    } else {
      // we have to go clock wise to get to stopper
      a_motor->state = ARM_MOTOR_CALIBRATION_HOLD_NEG_SPEED;
      set_motor_speed(a_motor->index, -(a_motor->calibration_speed));
    }

    a_motor->moving_time_ms = 0;
    log_message(LOG_INFO, "Starting %s calibration\n", a_motor->name);
    break;

  case ARM_MOTOR_CALIBRATION_HOLD_POS_SPEED:
    if (a_motor->moving_time_ms > CALIBRATE_MOVE_HOLD_DURATION) {
      a_motor->state = ARM_MOTOR_CALIBRATE_POS_SPEED;
    }
    break;

  case ARM_MOTOR_CALIBRATE_POS_SPEED:
    if (check_stopped(a_motor,
                      CALIBRATION_STOP_DURATION)) { // if motor has stopped
      a_motor->stopper_pos = a_motor->motor->abs_pos;
      log_message(LOG_INFO, "Determined %s stop position: %ld\n", a_motor->name,
                  a_motor->stopper_pos);

      a_motor->state = ARM_MOTOR_CHECK_POSITION;
      a_motor->is_calibrated = true;
      set_motor_speed(a_motor->index, 0);
    }
    break;

  case ARM_MOTOR_CALIBRATION_HOLD_NEG_SPEED:
    if (a_motor->moving_time_ms > CALIBRATE_MOVE_HOLD_DURATION) {
      a_motor->state = ARM_MOTOR_CALIBRATE_NEG_SPEED;
    }
    break;

  case ARM_MOTOR_CALIBRATE_NEG_SPEED:
    if (check_stopped(a_motor,
                      CALIBRATION_STOP_DURATION)) { // if motor has stopped
      a_motor->stopper_pos = a_motor->motor->abs_pos;
      log_message(LOG_INFO, "Determined %s stop position: %ld\n", a_motor->name,
                  a_motor->stopper_pos);

      a_motor->state = ARM_MOTOR_CHECK_POSITION;
      a_motor->is_calibrated = true;
      set_motor_speed(a_motor->index, 0);
    }
    break;

  default:
    return -1;
    break;
  }

  a_motor->moving_time_ms++;
  return a_motor->state;
}

/**
 * @brief Function to update if movement was recorded this call and return if
 * true if has not moved in the last 16 calls
 * @author Luke Cartier
 * @param a_motor
 * @return bool
 */
bool check_stopped(arm_motor_t *s_motor, uint16_t duration) {
  return s_motor->motor->stopped_duration >= duration;
}
