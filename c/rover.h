#ifndef ROVERCORE_ROVER_H
#define ROVERCORE_ROVER_H

#include "kria_mmio.h"
#include <aio.h>
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

void rover_move_x(MotorController *rover, int distance);
void rover_move_y(MotorController *rover, int distance);
void rover_rotate(MotorController *rover, int angle);

#endif // ROVERCORE_ROVER_H
