#ifndef PYNQ_ARM_MOTOR_CONTROLLER_H
#define PYNQ_ARM_MOTOR_CONTROLLER_H

#include "pynq_mmio.h"
#include <stdint.h>

#define WRIST_MOTOR_IDX 0
#define ELBOW_MOTOR_IDX 1
#define BASE_MOTOR_IDX 2
#define CLAW_MOTOR_IDX 3

#define WRIST_MOTOR_PIN 0
#define ELBOW_MOTOR_PIN 1
#define BASE_MOTOR_PIN 2
#define CLAW_MOTOR_PIN 3

#define MOTOR_READ_REG 0x30
#define MOTOR_WRITE_REG 0x20

#define MAX_MOTORS 4
#define MOTOR_DATA_SIZE 8

#define UPPER_MASK 0xFFFFFF00
#define LOWER_MASK 0x000000FF

#define HIGH_THRESH 206 // 256-50
#define LOW_THRESH 50

#define POSITION_FIFO_SIZE 10

typedef struct {
  uint8_t pin;
  long abs_pos;
  long long target_pos;
  int velocity;
  uint8_t raw_pos;
  long position_fifo[POSITION_FIFO_SIZE];
  uint8_t position_fifo_idx;
  uint16_t stopped_duration; // time in ms motor has been stopped
} motor_t;

void motor_init();
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