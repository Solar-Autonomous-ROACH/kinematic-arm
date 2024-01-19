#include "rover.h"

bool rover_movement_done() {
  log_message(LOG_INFO, "CHECKING IF ROVER IS DONE MOVING\n");
  return true;
}

void tank_turn(int16_t target_angle) {
  log_message(LOG_INFO, "PERFORMING TANK TURN, ROTATING %s DEGREES\n",
              target_angle);
}

void move_forward(int16_t dist) {
  log_message(LOG_INFO, "MOVING FORWARD BY DISTANCE %s CENTIMETERS\n", dist);
}

void move_right(int16_t dist) {
  log_message(LOG_INFO, "MOVING RIGHT BY DISTANCE %s CENTIMETERS\n", dist);
}