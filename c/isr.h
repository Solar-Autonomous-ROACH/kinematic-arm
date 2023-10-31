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

#define BASE_HOME_ANGLE 0
#define ELBOW_HOME_ANGLE 0
#define WRIST_HOME_ANGLE 190

#define BASE_PLACE_ANGLE 0
#define ELBOW_PLACE_ANGLE 0
#define WRIST_PLACE_ANGLE 0

#define WRIST_PREP_ANGLE 150

typedef enum {
  CALIBRATE,
  WAIT_FOR_INPUT,
  PREPARE_TO_MOVE,
  MOVE_TARGET,
  CLAW_ACQUIRE,
  PLACE_TARGET,
  MOVE_HOME,
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
