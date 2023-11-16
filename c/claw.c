#include "claw.h"
#include "arm.h"

claw_state_t claw_handle_state(claw_motor_t * c_motor) {
  switch (c_motor->state) {
  case IDLE:
    if (claw_rotate_ready()){
      c_motor->state = ROTATE;
    }
    break;
  case ROTATE:
    if (claw_rotation_complete()){
        c_motor->state = CLOSE;
    }
    break;
  case CLOSE:
    if (claw_close()){
        c_motor->state = ACQUIRED;
    }
    break;
  case ACQUIRED:
    if (claw_open()){
      c_motor->state = IDLE;
      set_claw_rotate_ready(false);
    }
    
    break;
  default:
    break;
  }
  return c_motor->state;
}

void set_claw_angle(uint16_t angle) {
  CLAW_MOTOR.motor->target_pos = angle; // probably have to change this
}

bool claw_open(void){
  return true;
}

bool claw_close(void){
  return object_acquired();
}

bool claw_rotation_complete(claw_motor_t * c_motor){
  long current_position = c_motor->motor->abs_pos;
  long target_position = c_motor->motor->target_pos;
  long diff = target_position - current_position;
  long abs_diff = diff < 0 ? -diff : diff;
  if (abs_diff < CLAW_ERROR_MARGIN){
    set_motor_speed(c_motor->motor->index, 0);
    return true;
  }
  else {
    set_motor_speed(c_motor->motor->index, diff > 0 ? CLAW_SPEED : -CLAW_SPEED);
    return false;
  }
}