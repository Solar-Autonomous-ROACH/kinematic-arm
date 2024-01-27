#ifndef PYNQ_ROVER_H
#define PYNQ_ROVER_H

#include "logger.h"
#include <aio.h>
#include <stdbool.h>
#include <stdint.h>

bool rover_movement_done();
void rover_rotate(int16_t turn_angle);
void rover_move_x(int16_t dist);
void rover_move_y(int16_t dist);

#endif