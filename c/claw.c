#include "claw.h"
#include "logger.h"
// negative claw speed = rotation
// positive claw speed = open/close
claw_state_t claw_handle_state(claw_motor_t *c_motor) {
  long diff;
  long abs_diff;

  switch (c_motor->state) {
  case CLAW_CHECK_POSITION:
    diff = c_motor->target_angle_ticks - c_motor->current_angle_ticks;
    abs_diff = diff < 0 ? -diff : diff;
    if (abs_diff > CLAW_ERROR_MARGIN) {
      // assuming turning negative turns the claw
      if (diff <= 0) {
        // positive angle->we can directly turn to the difference
        c_motor->motor->target_pos = c_motor->motor->abs_pos - abs_diff;
      } else {
        long full_turn = c_motor->CPR * c_motor->gear_ratio;
        c_motor->motor->target_pos =
            c_motor->motor->abs_pos - (full_turn - abs_diff);
      }
      set_motor_speed(c_motor->index, -CLAW_SPEED);
      c_motor->state = CLAW_ROTATING;
    } else if (c_motor->target_is_open != c_motor->is_open) {
      // assume turning positive opens/closes the claw
      long turn_90_deg = c_motor->CPR * c_motor->gear_ratio / 4;
      c_motor->motor->target_pos = c_motor->motor->abs_pos + turn_90_deg;
      c_motor->state = CLAW_OPENING_CLOSING;
      set_motor_speed(c_motor->index, CLAW_SPEED);
    }
    break;

  case CLAW_ROTATING:
    // this diff has to be non-positive!
    diff = c_motor->motor->target_pos - c_motor->motor->abs_pos;
    // update current_angle_ticks
    if (diff >= -CLAW_ERROR_MARGIN) {
      // we are there!
      set_motor_speed(c_motor->index, 0);
      c_motor->current_angle_ticks = c_motor->target_angle_ticks; // TODO: add a -diff
      // check if we need to open/close claw
      if (c_motor->target_is_open != c_motor->is_open) {
        // assume turning positive opens/closes the claw
        long turn_90_deg = c_motor->CPR * c_motor->gear_ratio / 4;
        c_motor->motor->target_pos = c_motor->motor->abs_pos + turn_90_deg;
        c_motor->state = CLAW_OPENING_CLOSING;
        set_motor_speed(c_motor->index, CLAW_SPEED);
      } else {
        c_motor->state = CLAW_CHECK_POSITION;
      }
    }
    break;


  case CLAW_OPENING_CLOSING:
    diff = c_motor->motor->target_pos - c_motor->motor->abs_pos;
    if (diff <= CLAW_ERROR_MARGIN) {
      // we are there!
      c_motor->is_open = c_motor->target_is_open;
      set_motor_speed(c_motor->index, 0);
      c_motor->state = CLAW_CHECK_POSITION;
    }
    break;

  default:
    break;
  }
  return c_motor->state;
}

void set_claw_angle(claw_motor_t *c_motor, uint16_t angle) {
  if (c_motor->state == CLAW_CHECK_POSITION) {
    long ticks = angle * c_motor->CPR * c_motor->gear_ratio / 360;
    c_motor->target_angle_ticks = ticks;
  }
}

void open_claw(claw_motor_t *c_motor) {
  if (c_motor->state == CLAW_CHECK_POSITION) {
    c_motor->target_is_open = true;
  }
}

void close_claw(claw_motor_t *c_motor) {
  if (c_motor->state == CLAW_CHECK_POSITION) {
    c_motor->target_is_open = false;
  }
}

// bool claw_rotation_complete(claw_motor_t *c_motor) {
//   long current_position = c_motor->motor->abs_pos;
//   long target_position = c_motor->motor->target_pos;
//   long diff = target_position - current_position;
//   long abs_diff = diff < 0 ? -diff : diff;
//   if (abs_diff < CLAW_ERROR_MARGIN) {
//     set_motor_speed(c_motor->index, 0);
//     return true;
//   } else {
//     set_motor_speed(c_motor->index, diff > 0 ? CLAW_SPEED : -CLAW_SPEED);
//     return false;
//   }
// }