#include "claw.h"
#include "arm.h"

claw_state_t claw_handle_state(claw_motor_t * c_motor) {
  switch (c_motor->state) {
  case IDLE:
    c_motor->state = ROTATE;
    break;
  case ROTATE:
    if (true/*at position*/){
        //close claw
        c_motor->state = CLOSE;
    }
    break;
  case CLOSE:
    if (object_acquired()){
        c_motor->state = ACQUIRED;
    }
    break;
  case ACQUIRED:
    break;
  default:
    break;
  }
  return c_motor->state;
}

void set_claw_angle(uint16_t angle) {
  CLAW_MOTOR.motor->target_pos = angle; // probably have to change this
}