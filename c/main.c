//
// Created by Tyler Bovenzi on 3/23/23.
//

#include "arm.h"
#include "kinematic_engine.h"
#include "rover_isr.h"

#include "rover.h"
#include "vision.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void sigint_handler(int sig) {
  printf("Received SIGINT signal %d\n", sig);
  arm_close();
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

  vision_init();
  arm_init();
  isr_init();

  while (1) {
    pause();
  }
}
