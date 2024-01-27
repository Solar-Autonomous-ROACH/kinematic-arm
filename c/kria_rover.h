#ifndef KRIA_ROVER_H
#define KRIA_ROVER_H

#include "kria_mmio.h"
#include "logger.h"
#include <aio.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  // MMIO
  volatile unsigned int *mmio;

  // GPIO Output Control Signals
  uint8_t duty_cycle;  // 8 bits
  uint8_t clk_divisor; // 3 bits
  uint8_t dir;         // 1 bit
  uint8_t en_motor;    // 1 bit
  uint8_t clear_enc;   // 1 bit
  uint8_t en_enc;      // 1 bit

  // GPIO Input Control Signals
  int16_t counts; // 16 bits

} MotorController;

int MotorController_init(MotorController *motor, off_t mmio_address);
void MotorController_close(MotorController *motor);
void MotorController_write(MotorController *motor);
void MotorController_read(MotorController *motor);
void MotorController_set_speed(MotorController *motor, int speed);

bool rover_movement_done();
void rover_rotate(int16_t turn_angle);
void rover_move_x(int16_t dist);
void rover_move_y(int16_t dist);

#endif