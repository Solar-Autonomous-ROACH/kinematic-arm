//
// Created by Tyler Bovenzi on 3/23/23.
//

#include "arm.h"
#include "isr.h"
#include "kinematic_engine.h"
#include "led.h"
#include "mmio.h"
#include "vision.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void sigint_handler(int sig) {
  printf("Received SIGINT signal %d\n", sig);
  if (mmio_is_valid()) {
    for (int i = 0; i < 14; i++) {
      set_motor_speed(i, 0);
    }
  }
  set_brightness(100, 000, 100);
  vision_terminate(true);
  exit(0);
}

int main() {
  struct sigaction sa;
  sa.sa_handler = sigint_handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  sigaddset(&sa.sa_mask, SIGALRM);
  sigaction(SIGINT, &sa, NULL);

  mmio_init();
  printf("MMIO INIT DONE\n");
  vision_init();
  isr_init();
  set_led_status();
  set_brightness(100, 100, 000);

  pause();

  close_mem();
}
