#ifndef ROVERCORE_ROVER_H
#define ROVERCORE_ROVER_H

#include "logger.h"
#include <stdbool.h>
#include <stdint.h>

bool rover_movement_done();

void tank_turn(int16_t target_angle);

void move_forward(int16_t dist);

void move_right(int16_t dist);

#endif
