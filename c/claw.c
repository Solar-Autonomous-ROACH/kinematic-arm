#include "claw.h"

claw_state_t claw_handle_state(claw_motor_t * c_motor) {
  switch (c_motor->state) {
  case ROTATE_TARGET:
    if (claw_rotation_complete(c_motor)){
        c_motor->state = CLOSE;
    }
    break;
  case CLOSE:
    if (claw_close()){
      set_claw_angle(c_motor, 0);
      c_motor->state = ROTATE_ZERO_AND_OPEN;
    }
    break;

  case ROTATE_ZERO_AND_OPEN:
    if (claw_rotation_complete(c_motor) && claw_open()){
      c_motor->state = ROTATE_TARGET;
    }
    
    break;
  default:
    break;
  }
  return c_motor->state;
}

void set_claw_angle(claw_motor_t * c_motor, uint16_t angle) {
  c_motor->motor->target_pos = angle; //not sure if this is right
}

bool claw_open(void){
  return true;
}

bool claw_close(void){
  return true;
}

bool claw_rotation_complete(claw_motor_t * c_motor){
  long current_position = c_motor->motor->abs_pos;
  long target_position = c_motor->motor->target_pos;
  long diff = target_position - current_position;
  long abs_diff = diff < 0 ? -diff : diff;
  if (abs_diff < CLAW_ERROR_MARGIN){
    set_motor_speed(c_motor->index, 0);
    return true;
  }
  else {
    set_motor_speed(c_motor->index, diff > 0 ? CLAW_SPEED : -CLAW_SPEED);
    return false;
  }
}