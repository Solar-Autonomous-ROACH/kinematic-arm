// TEST MMIO GPIO CONTROL
#include "rover.h"
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#define HALL_EFFECT_ADDRESS 0x80110000

static volatile bool done = false;
static volatile bool detected = true;
static volatile unsigned int *hall_mmio;
static MotorController claw_motor;

void sigint_handler() {
  printf("SIGINT caught\n");
  done = true;
}

long long timems() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (long long)tv.tv_sec * 1000LL + tv.tv_usec / 1000;
}

void read_hall_effect() {
  static int print_count = 1000;
  static int count = 1000;
  unsigned int reading = *hall_mmio;
  if (detected && count > 2000) {
    claw_motor.dir = 1; // 0 open/close
    claw_motor.duty_cycle = 20 << 1;
    MotorController_write(&claw_motor);
    detected = false;
    count = 0;
  } else if (!detected) {
    if (count > 200 && reading == 0) {
      claw_motor.duty_cycle = 0;
      MotorController_write(&claw_motor);
      detected = true;
      count = 0;
    }
  }
  if (print_count == 1000) {
    printf("Hall effect %d\n", *hall_mmio);
    print_count = 0;
  }
  count++;
  print_count++;
}

void read_hall_effect2() {
  static time_t print_time;
  static long long change_time;
  unsigned int reading = *hall_mmio;
  if (detected && (timems() - change_time) > 2000) {
    claw_motor.dir = 1; // 0 open/close
    claw_motor.duty_cycle = 20 << 1;
    MotorController_write(&claw_motor);
    detected = false;
    change_time = timems();
  } else if (!detected) {
    if (timems() - change_time > 100 && reading == 0) {
      claw_motor.duty_cycle = 0;
      MotorController_write(&claw_motor);
      detected = true;
      change_time = timems();
    }
  }
  if (time(NULL) - print_time >= 1) {
    printf("Hall effect %d\n", *hall_mmio);
    print_time = time(NULL);
  }
}

int main() {
  // Configure signal handler
  signal(SIGINT, sigint_handler);
  hall_mmio = mmio_init(0x80110000);
  if (MotorController_init(&claw_motor, MOTOR_CLAW) == -1) {
    fprintf(stderr, "Mission failed\n");
    return -1;
  }
  claw_motor.clk_divisor = 2;

  // Initialize rover
  // if (rover_init() != 0) {
  //   printf("failed to initialize rover\n");
  //   return -1;
  // }
  // isr_attach_function(read_hall_effect);
  if (isr_init() != 0) {
    printf("failed to initialize rover\n");
    return -1;
  }

  // claw_motor.dir = 0; // 0 open/close

  // testing
  // motor_set_speed(MOTOR_MIDDLE_LEFT_WHEEL, 1000);
  // rover_steer_forward();
  // rover_move_x(-10000, 128);

  // infinite loop
  while (!done) {
    read_hall_effect2();
  }
  printf("Exiting...\n");

  // Close Rover
  // if (rover_close() != 0) {
  //   printf("failed to close rover\n");
  //   return -1;
  // }
  MotorController_close(&claw_motor);
  close_mem(hall_mmio);

  return 0;
}
