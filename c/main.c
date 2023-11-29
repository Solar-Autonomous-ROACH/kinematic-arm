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
  int16_t input1, input2, input3, input4;
  int16_t calc_base_angle, calc_elbow_angle, calc_wrist_angle, turn_angle, claw_angle;
  while (1) {
    // scanf("%d", &speed1);
    char c = getchar();
    if (c < 0) {
      continue;
    }
    switch (c) {
    case 'H':
      validate_angle_set(0, 0, 0, 0);
      break;

    case 'A':
      if (scanf("%hd %hd %hd %hd\n", &input1, &input2, &input3, &input4) >
          0) {
        validate_angle_set(input1, input2, input3, input4);
      }
      break;

    case 'C':
      if (scanf("%hd %hd %hd %hd\n", &input1, &input2, &input3, &input4) >
          0) {
        kinematic_engine(input1, input2, input3, &calc_base_angle,
                         &calc_elbow_angle, &calc_wrist_angle, &turn_angle);
        claw_angle = input4;
        validate_angle_set(calc_base_angle, calc_elbow_angle, calc_wrist_angle, claw_angle);
      }
      break;

    default:
      break;
    }
  }

  close_mem();
}
