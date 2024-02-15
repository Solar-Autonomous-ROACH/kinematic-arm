#ifndef KRIA_ROVER_H
#define KRIA_ROVER_H

#include "kria_mmio.h"
#include "logger.h"
#include <aio.h>
#include <stdbool.h>
#include <stdint.h>

#define GPIO_READ_NOT_IMPLEMENTED 0xFF
#define GPIO_HALL_EFFECT_PIN 0

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

uint8_t gpio_read(uint16_t pin);

bool check_rover_done();
void rover_rotate(int dir, int angle);
void rover_move_x(int64_t dist, double speed);

#endif