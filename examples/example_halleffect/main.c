// TEST MMIO GPIO CONTROL
#include "rover.h"
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define HALL_EFFECT_ADDRESS 0x80110000

static bool done = false;
static volatile int *hall_mmio;

void sigint_handler() {
  printf("SIGINT caught\n");
  done = true;
}

void read_hall_effect() {
  static int count = 1000;
  if (count == 1000) {
    printf("Hall effect %d\n", *hall_mmio);
    count = 0;
  }
  count++;
}

int main() {
  // Configure signal handler
  signal(SIGINT, sigint_handler);
  hall_mmio = mmio_init(0x80110000);
  // Initialize rover
  // if (rover_init() != 0) {
  //   printf("failed to initialize rover\n");
  //   return -1;
  // }
  isr_attach_function(read_hall_effect);
  if (isr_init() != 0) {
    printf("failed to initialize rover\n");
    return -1;
  }

  // testing
  // motor_set_speed(MOTOR_MIDDLE_LEFT_WHEEL, 1000);
  // rover_steer_forward();
  // rover_move_x(-10000, 128);

  // infinite loop
  while (done == false) {
  }

  // Close Rover
  if (rover_close() != 0) {
    printf("failed to close rover\n");
    return -1;
  }
  close_mem(hall_mmio);

  return 0;
}
