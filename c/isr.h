//
// Created by Tyler Bovenzi on 3/23/23.
//

#include <signal.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include "mmio.h"
#include "motor.h"
#include "move.h"
#include "stdint.h"
// #include "rover.h"
// #include "steering_motor.h"
#include "led.h"

#ifndef ROVERCORE_ISR_H
#define ROVERCORE_ISR_H

typedef enum {
  CALIBRATE,
  WAIT_FOR_INPUT,
  MOVE,
  CLAW_ACQUIRE,
  PLACE_TARGET,
} arm_state_t;

typedef enum {
  IDLE,
  ROTATE,
  OPEN,
  CLOSE,
} claw_state_t;

int speed1;

int isr_init();

int isr(int signum);

#endif // ROVERCORE_ISR_H
