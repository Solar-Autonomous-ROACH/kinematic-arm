//
// Created by Tyler Bovenzi on 3/23/23.
//

#include <arm.h>
#include <rover.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <vision.h>

#define ARM_ONLY

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
  arm_set_rover_funcs(false);
  vision_init();
  arm_init();
  isr_init();

  while (1) {
    // rover_steer_forward();
    // rover_move_x(100);
    // while (!rover_move_done()) {
    //   /* block */
    // }
    arm_begin_pickup();
    while (!arm_pickup_done()) {
      /* block */
    }
    pause();
  }
}
