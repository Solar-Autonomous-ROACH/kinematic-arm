#ifndef ROVERCORE_ROVER_H
#define ROVERCORE_ROVER_H

#include "logger.h"
#include <stdbool.h>
#include <stdint.h>

bool rover_movement_done();

void rover_rotate(int16_t turn_angle);

void rover_move_x(int16_t dist);

#endif
