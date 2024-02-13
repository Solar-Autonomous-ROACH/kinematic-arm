#include "claw.h"
#include "logger.h"

claw_motor_t CLAW_MOTOR;

#define MAGNET_DETECTED(reading) (reading == 0) // you love to see it

void claw_init() {
  CLAW_MOTOR.index = CLAW_MOTOR_IDX;
  CLAW_MOTOR.motor =
      get_motor(CLAW_MOTOR_PIN); // TODO: Change to correct motor value
  CLAW_MOTOR.state = CLAW_CHECK_POSITION;
  CLAW_MOTOR.CPR = 12;
  CLAW_MOTOR.gear_ratio = 19.225;
  CLAW_MOTOR.current_angle_ticks = 0; // assume claw is at 0 degrees and opened
  CLAW_MOTOR.is_open = true;
  CLAW_MOTOR.target_angle_ticks = 0;
  CLAW_MOTOR.target_is_open = true;
}

// negative claw speed = rotation
// positive claw speed = open/close
claw_state_t claw_handle_state() {
  long diff;
  long abs_diff;
  uint8_t hall_reading = gpio_read(GPIO_HALL_EFFECT_PIN);

  switch (CLAW_MOTOR.state) {
  case CLAW_CALIBRATE_START:
    set_claw_speed(0, false);
    if (hall_reading == GPIO_READ_NOT_IMPLEMENTED) {
      // gpio_read not implemented
      CLAW_MOTOR.state = CLAW_CHECK_POSITION; // assume calibrated
    } else if (MAGNET_DETECTED(hall_reading)) {
      // magnet detected
      toggle_claw(); // opens/close claw
      // open/close claw to see which magnet was detected
      CLAW_MOTOR.state = CLAW_CALIBRATE_WAIT_FOR_TURN;
    } else {
      // turn whole claw to start looking for magnet
      set_claw_speed(CLAW_SPEED, false);
      CLAW_MOTOR.state = CLAW_CALIBRATE_WAIT_FOR_MAGNET;
    }
    break;

  case CLAW_CALIBRATE_WAIT_FOR_MAGNET:
    if (MAGNET_DETECTED(hall_reading)) {
      // magnet detected
      set_claw_speed(0, false);
      // open/close claw to see which magnet was detected
      toggle_claw();
      CLAW_MOTOR.state = CLAW_CALIBRATE_WAIT_FOR_TURN;
    }
    // no magnet detected. keep going
    break;

  case CLAW_CALIBRATE_WAIT_FOR_TURN:
    diff = CLAW_MOTOR.target_angle_ticks - CLAW_MOTOR.current_angle_ticks;
    abs_diff = diff < 0 ? -diff : diff;
    break;

  case CLAW_CHECK_POSITION:
    diff = CLAW_MOTOR.target_angle_ticks - CLAW_MOTOR.current_angle_ticks;
    abs_diff = diff < 0 ? -diff : diff;
    if (abs_diff > CLAW_ERROR_MARGIN) {
      long ticks;
      // assuming turning negative turns the claw
      if (diff <= 0) {
        // positive angle->we can directly turn to the difference
        ticks = abs_diff;
      } else {
        // TODO: christian: cam this be changed to -180 + abs diff instead of
        // 360 - abs diff aka ticks = - CLAW_MOTOR.CPR * CLAW_MOTOR.gear_ratio /
        // 2 + abs_diff;
        ticks = CLAW_MOTOR.CPR * CLAW_MOTOR.gear_ratio - abs_diff;
      }
      claw_relative_turn(ticks, false);
      CLAW_MOTOR.state = CLAW_ROTATING;
    } else if (CLAW_MOTOR.target_is_open != CLAW_MOTOR.is_open) {
      toggle_claw();
    }
    break;

  case CLAW_ROTATING:
    // update current_angle_ticks
    if (claw_turn_done(NULL, false)) {
      // we are there!
      set_claw_speed(0, false);
      CLAW_MOTOR.current_angle_ticks =
          CLAW_MOTOR.target_angle_ticks; // TODO: add a -diff
      // check if we need to open/close claw
      if (CLAW_MOTOR.target_is_open != CLAW_MOTOR.is_open) {
        toggle_claw();
      } else {
        CLAW_MOTOR.state = CLAW_CHECK_POSITION;
      }
    }
    break;

  case CLAW_OPENING_CLOSING:
    diff = CLAW_MOTOR.motor->target_pos - CLAW_MOTOR.motor->abs_pos;
    if (diff <= CLAW_ERROR_MARGIN) {
      // we are there!
      CLAW_MOTOR.is_open = CLAW_MOTOR.target_is_open;
      set_claw_speed(0, false);
      CLAW_MOTOR.state = CLAW_CHECK_POSITION;
    }
    break;

  default:
    break;
  }
  return CLAW_MOTOR.state;
}

void claw_goto_calibrate() { CLAW_MOTOR.state = CLAW_CALIBRATE_START; }

void set_claw_angle(uint16_t angle) {
  log_message(LOG_INFO, "setting claw angle to %d\n", angle);
  if (CLAW_MOTOR.state == CLAW_CHECK_POSITION) {
    long ticks = angle * CLAW_MOTOR.CPR * CLAW_MOTOR.gear_ratio / 360;
    CLAW_MOTOR.target_angle_ticks = ticks;
  }
}

void open_claw() {
  if (CLAW_MOTOR.state == CLAW_CHECK_POSITION) {
    CLAW_MOTOR.target_is_open = true;
  }
}

void close_claw() {
  if (CLAW_MOTOR.state == CLAW_CHECK_POSITION) {
    CLAW_MOTOR.target_is_open = false;
  }
}

/**
 * @brief Set the claw speed object
 *
 * @param speed
 * @param openclose if true open/close claw. false rotates everything
 * openclose doesn't matter if speed is 0
 */
void set_claw_speed(uint8_t speed, bool openclose) {
  // assume turning positive opens/closes the claw
  // turning positive should increase abs_pos
  set_motor_speed(CLAW_MOTOR.index, openclose ? speed : -speed);
}

bool claw_turn_done(long *diff_pt, bool openclose) {
  long diff = CLAW_MOTOR.motor->target_pos - CLAW_MOTOR.motor->abs_pos;

  if (openclose && diff <= CLAW_ERROR_MARGIN ||
      !openclose && diff >= -CLAW_ERROR_MARGIN) {
    if (diff_pt) {
      *diff_pt = diff;
    }
    return true;
  }
  return false;
}

/**
 * @brief Turn the claw ticks from current position
 *
 * @param ticks
 * @param openclose
 */
void claw_relative_turn(uint16_t ticks, bool openclose) {
  CLAW_MOTOR.motor->target_pos = openclose ? CLAW_MOTOR.motor->abs_pos + ticks
                                           : CLAW_MOTOR.motor->abs_pos - ticks;

  long abs_diff =
      labs(CLAW_MOTOR.motor->target_pos - CLAW_MOTOR.motor->abs_pos);

  if (abs_diff > CLAW_ERROR_MARGIN) {
    set_claw_speed(CLAW_SPEED, openclose);
  }
}

void toggle_claw() {
  long turn_90_deg = CLAW_MOTOR.CPR * CLAW_MOTOR.gear_ratio / 4;
  CLAW_MOTOR.state = CLAW_OPENING_CLOSING;
  claw_relative_turn(turn_90_deg, true);
}
