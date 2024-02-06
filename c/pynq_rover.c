#include "pynq_rover.h"

uint8_t gpio_read(uint16_t pin __attribute__((unused))) {
  return GPIO_READ_NOT_IMPLEMENTED; // TODO: get implementation
}

bool rover_movement_done() {
  log_message(LOG_INFO, "CHECKING IF ROVER IS DONE MOVING\n");
  return true;
}

void rover_rotate(int16_t turn_angle) {
  if (turn_angle > 90 || turn_angle < -90) {
    log_message(LOG_ERROR, "Invalid turn angle\n");
  } else {
    log_message(LOG_INFO, "Rover rotating to target angle %d\n", turn_angle);
  }
}

void rover_move_x(int16_t dist) {
  log_message(LOG_INFO, "Rover moving %s by distance %d\n",
              dist > 0 ? "forward" : "backward", dist > 0 ? dist : -dist);
}

void rover_move_y(int16_t dist) {
  log_message(LOG_INFO, "Rover moving %s by distance %d\n",
              dist > 0 ? "right" : "left", dist > 0 ? dist : -dist);
}
