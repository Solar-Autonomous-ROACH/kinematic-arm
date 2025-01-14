// TEST MMIO GPIO CONTROL
#include <arm.h>
#include <math.h>
#include <rover.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <vision.h>

void sigint_handler() {
  printf("SIGINT caught\n");
  // Close Rover
  vision_terminate(true);
  arm_close();
  if (rover_close() != 0) {
    fprintf(stderr, "failed to close rover\n");
  }
  exit(0);
}

int main() {
  // Configure signal handler
  struct sigaction sa;
  sa.sa_handler = sigint_handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  sigaddset(&sa.sa_mask, SIGALRM);
  sigaction(SIGINT, &sa, NULL);

  // Initialize rover
  if (rover_init() != 0) {
    printf("failed to initialize rover\n");
    return -1;
  }
  vision_init();
  arm_init();
  if (isr_init() != 0) {
    printf("failed to initialize rover\n");
    return -1;
  }

  while (rover_is_calibrated() == false) {
  }

  // testing
  // motor_set_speed(MOTOR_MIDDLE_LEFT_WHEEL, 1000);
  // rover_steer_forward();
  // rover_move_x(-10000, 128);

  while (1) {
    arm_begin_pickup();
    while (!arm_pickup_done()) {
      /* block */
    }
  }

  return 0;
}