#ifndef PYNQ_ROVER_H
#define PYNQ_ROVER_H

#include "logger.h"
#include <aio.h>
#include <stdbool.h>
#include <stdint.h>

#define GPIO_READ_NOT_IMPLEMENTED 0xFF
#define GPIO_HALL_EFFECT_PIN 0

uint8_t gpio_read(uint16_t pin);

bool rover_movement_done();
void rover_rotate(int16_t turn_angle);
void rover_move_x(int16_t dist);

#endif