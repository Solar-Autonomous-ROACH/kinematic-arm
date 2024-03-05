// TEST MMIO GPIO CONTROL
#include "arm.h"
#include "rover.h"
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

void sigint_handler() {
  printf("SIGINT caught\n");
  // Close Rover
  arm_close();
  vision_terminate(true);
  if (rover_close() != 0) {
    fprintf(stderr, "failed to close rover\n");
  }
  exit(0);
}

int main() {
  // Configure signal handler
  signal(SIGINT, sigint_handler);
  // Initialize rover
  if (rover_init() != 0) {
    printf("failed to initialize rover\n");
    return -1;
  }
  arm_init();
  if (isr_init() != 0) {
    printf("failed to initialize rover\n");
    return -1;
  }

  // testing
  // motor_set_speed(MOTOR_MIDDLE_LEFT_WHEEL, 1000);
  rover_move_x(-10000, 128);

  // infinite loop
  while (1) {
    pause();
  }

  return 0;
}