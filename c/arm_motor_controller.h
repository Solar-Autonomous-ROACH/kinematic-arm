#ifndef ARM_MOTOR_CONTROLLER_H
#define ARM_MOTOR_CONTROLLER_H

#include "rover.h"
#include <aio.h>
#include <stdint.h>
#include <stdio.h>

#define WRIST_MOTOR_IDX 0
#define ELBOW_MOTOR_IDX 1
#define BASE_MOTOR_IDX 2
#define CLAW_MOTOR_IDX 3

// Motors MMIO Addresses
#define BASE_MOTOR_ADDRESS MOTOR_FRONT_RIGHT_STEER
#define ELBOW_MOTOR_ADDRESS MOTOR_REAR_RIGHT_WHEEL
#define WRIST_MOTOR_ADDRESS MOTOR_REAR_RIGHT_STEER
#define CLAW_MOTOR_ADDRESS MOTOR_FRONT_RIGHT_WHEEL

#define MAX_MOTORS 4
#define ENCODER_RESOLUTION_NUM_BITS 16
#define ENCODER_RESOLUTION_TICKS 65536

#define POSITION_FIFO_SIZE 10

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

void motor_init();
int motor_update(uint8_t motor_index);
void motor_update_all();

int set_motor_speed(uint8_t motor_index, int speed);

long get_motor_position(uint8_t motor_index);

long long get_target_position(uint8_t motor_index);

int set_target_position(uint8_t motor_index, long long target_position);

int get_motor_velocity(uint8_t motor_index);
motor_t *get_motor(uint8_t motor_index);

void set_motor_position(uint8_t motor_index, long position);
long get_motor_position_n(uint8_t motor_index, uint8_t n);

#endif // ROVERCORE_MOTOR_H
