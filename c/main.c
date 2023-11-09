//
// Created by Tyler Bovenzi on 3/23/23.
//

#include "arm.h"
#include "isr.h"
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
  int16_t base_in_angle, elbow_in_angle, wrist_in_angle, claw_in_angle;
  while (1) {
    // scanf("%d", &speed1);
    //if (Flag is set by vision team)
    if (scanf("%hd %hd %hd %hd\n", &base_in_angle, &elbow_in_angle,
              &wrist_in_angle, &claw_in_angle) > 0) {
      validate_angle_set(base_in_angle, elbow_in_angle, wrist_in_angle, claw_in_angle);
      printf("HERE");
    }
  }

  close_mem();
}
