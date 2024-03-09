#include "arm_motor.h"
#include "logger.h"

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
    /** Before doing anything check if motor stopped moving
     * start checking after motor has been started for 200ms
     */
    if (abs_diff > MOTOR_TICKS_ERROR_MARGIN) {
      if (a_motor->moving_time_ms > MOTOR_STALL_START_CHECKING_TIME_MS &&
          check_stopped(a_motor, MOTOR_STALL_DURATION)) {
        // motor stopped for 10 ms
        log_message(LOG_WARNING, "%s collision detected\n", a_motor->name);
        set_motor_speed(a_motor->index, 0); // stop motor
        a_motor->state = ARM_MOTOR_ERROR;
      } else {
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
      }
    } else {
      a_motor->state = ARM_MOTOR_CHECK_POSITION;
      log_message(LOG_INFO_2, "%s reached position\n", a_motor->name);
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

  case ARM_MOTOR_ERROR:
    // don't do anything. main state machine should save us
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
    a_motor->stopper_pos = 0;
    a_motor->motor->stopped_duration = 0;
    a_motor->is_calibrated = false;

    if (!(a_motor->pos_angle)) {
      // we have to go counter clock wise to get to stopper
      set_motor_speed(a_motor->index, a_motor->calibration_speed);
    } else {
      // we have to go clock wise to get to stopper
      set_motor_speed(a_motor->index, -(a_motor->calibration_speed));
    }
    a_motor->state = ARM_MOTOR_CALIBRATE_IN_PROGRESS;

    a_motor->moving_time_ms = 0;
    log_message(LOG_INFO, "Starting %s calibration\n", a_motor->name);
    break;

  case ARM_MOTOR_CALIBRATE_IN_PROGRESS:
    if (a_motor->moving_time_ms > CALIBRATE_MOVE_HOLD_DURATION &&
        check_stopped(a_motor, CALIBRATION_STOP_DURATION)) {
      a_motor->stopper_pos = a_motor->motor->abs_pos;
      log_message(LOG_INFO, "Determined %s stop position: %ld\n", a_motor->name,
                  a_motor->stopper_pos);

      a_motor->state = ARM_MOTOR_CHECK_POSITION;
      a_motor->is_calibrated = true;
      set_motor_speed(a_motor->index, 0);
    }
    if (a_motor->moving_time_ms % 500 == 0) {
      log_message(LOG_DEBUG, "%s: Raw: %ld, Abs: %ld\n", a_motor->name,
                  a_motor->motor->raw_pos, a_motor->motor->abs_pos);
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
 * @brief Checks if the motor was stationary for duration
 * @param a_motor
 * @return bool
 */
bool check_stopped(arm_motor_t *a_motor, uint16_t duration) {
  return a_motor->motor->stopped_duration >= duration;
}

double get_motor_angle(arm_motor_t *a_motor) {
  return (get_motor_position(a_motor->index) - a_motor->stopper_pos) * 360 /
         (a_motor->CPR * a_motor->gear_ratio);
}
