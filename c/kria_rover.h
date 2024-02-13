#ifndef KRIA_ROVER_H
#define KRIA_ROVER_H

#include "kria_mmio.h"
#include "logger.h"
#include <aio.h>
#include <stdbool.h>
#include <stdint.h>

/* Define the Rover's Motors */
#define MOTOR_REAR_RIGHT_STEER 0x80000000
#define MOTOR_REAR_RIGHT_WHEEL 0x80010000

#define MOTOR_FRONT_RIGHT_STEER 0x80020000
#define MOTOR_FRONT_RIGHT_WHEEL 0x80030000

#define MOTOR_FRONT_LEFT_STEER 0x80040000
#define MOTOR_FRONT_LEFT_WHEEL 0x80050000

#define MOTOR_REAR_LEFT_STEER 0x80060000
#define MOTOR_REAR_LEFT_WHEEL 0x80070000

#define MOTOR_MIDDLE_LEFT_WHEEL 0x80080000
#define MOTOR_MIDDLE_RIGHT_WHEEL 0x80090000

#define MOTOR_WRIST 0x800A0000
#define MOTOR_BASE 0x800B0000
#define MOTOR_ELBOW 0x800C0000
#define MOTOR_CLAW 0x800D0000

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

bool rover_movement_done();
void rover_rotate(int16_t turn_angle);
void rover_move_x(int16_t dist);
void rover_move_y(int16_t dist);

#endif