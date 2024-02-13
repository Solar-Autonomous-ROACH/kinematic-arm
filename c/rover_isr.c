#include "rover_isr.h"
#include "arm.h"
#include "arm_motor.h"
#include "logger.h"

#include "arm_motor_controller.h"

static unsigned long millis; // stores number of milliseconds since startup
volatile unsigned int *watchdog_flag;

#define KRIA_BOARD

int isr_init() {
  struct sigaction sa;
  struct itimerval timer;

  // Install the ISR
  sa.sa_handler = (void *)isr;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGALRM, &sa, NULL);

  // Set the timer to trigger every 1ms
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 1000; // was 1000
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = 1000; // was 1000
  setitimer(ITIMER_REAL, &timer, NULL);
  set_target_position(0, 0);

  arm_init();
  log_message(LOG_INFO, "Initialization done\n");
  watchdog_flag = mmio_init((off_t)WATCHDOG_REG);
  return 0;
}

int isr(int signum __attribute__((unused))) {
  *(watchdog_flag) = *(watchdog_flag) ? 0 : 1;
  for (int i = 0; i < MAX_MOTORS; i++) {
    // printf("i: %d, ", i);
    motor_update(i);
  }
  // printf("\n");
// #define DEBUG_WRIST
#if defined(DEBUG_WRIST) || defined(DEBUG_ELBOW) || defined(DEBUG_BASE) ||     \
    defined(DEBUG_CLAW)
  arm_handle_state_debug();
#else
  arm_handle_state();
#endif
  // motor_update(CLAW_MOTOR_IDX);
  // set_motor_speed(WRIST_MOTOR_IDX, 50);
  // set_motor_speed(ELBOW_MOTOR_IDX, 40);

  millis++;
  return 0;
}
