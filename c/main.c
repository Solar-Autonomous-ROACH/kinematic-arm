//
// Created by Tyler Bovenzi on 3/23/23.
//

#include "arm.h"
#include "isr.h"
#include "kinematic_engine.h"
#include "led.h"
#include "mmio.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void sigint_handler(int sig) {
  printf("Received SIGINT signal %d\n", sig);
  if (mmio_is_valid()) {
    for (int i = 0; i < 10; i++) {
      set_motor_speed(i, 0);
    }
  }
  set_brightness(100, 000, 100);
  exit(0);
}

int main() {
  // int num;
  signal(SIGINT, sigint_handler);
  mmio_init();
  printf("MMIO INIT DONE\n");
  isr_init();
  set_led_status();
  set_brightness(100, 100, 000);
  speed1 = 0;
  int16_t calc_base_angle, calc_elbow_angle, calc_wrist_angle, calc_turn_angle;
  float x_in, y_in, z_in;
  while (1) {
    // scanf("%d", &speed1);
    if (scanf("%f %f %f\n", &x_in, &y_in, &z_in) > 0) {
      kinematic_engine(x_in, y_in, z_in, &calc_base_angle, &calc_elbow_angle,
                       &calc_wrist_angle, &calc_turn_angle);
      validate_angle_set(calc_base_angle, calc_elbow_angle, calc_wrist_angle);
      printf("HERE");
    }
  }

  close_mem();
}
