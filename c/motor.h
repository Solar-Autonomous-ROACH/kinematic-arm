#ifndef ROVERCORE_MOTOR_H
#define ROVERCORE_MOTOR_H

#include "mmio.h"
#include <aio.h>
#include <stdint.h>

#define MOTOR_READ_REG 0x30
#define MOTOR_WRITE_REG 0x20

#define MAX_MOTORS 16
#define MOTOR_DATA_SIZE 8

#define UPPER_MASK 0xFFFFFF00
#define LOWER_MASK 0x000000FF

#define KP 2
#define KV 2

#define POSITION_FIFO_SIZE 10
#define HIGH_THRESH 206 // 256-50
#define LOW_THRESH 50

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

typedef struct {
  long abs_pos;
  long long target_pos;
  int velocity;
  off_t MMIO_addr;
  uint16_t raw_pos;
  MotorController motor_controller;
  long position_fifo[POSITION_FIFO_SIZE];
  uint8_t position_fifo_idx;
  uint16_t stopped_duration; // time in ms motor has been stopped
} motor_t;

char get_raw_pos(uint8_t motor_index);

int motor_update(uint8_t motor_index);

int set_motor_speed(uint8_t motor_index, int speed);

long get_motor_position(uint8_t motor_index);

long long get_target_position(uint8_t motor_index);

int set_target_position(uint8_t motor_index, long long target_position);

int get_motor_velocity(uint8_t motor_index);
motor_t *get_motor(uint8_t motor_index);

void set_motor_position(uint8_t motor_index, long position);
long get_motor_position_n(uint8_t motor_index, uint8_t n);

#endif // ROVERCORE_MOTOR_H
