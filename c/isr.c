#include "isr.h"
#include "arm.h"
#include "arm_motor.h"
#include "logger.h"

#include "arm_motor_controller.h"
#include "mmio.h"

static unsigned long millis; // stores number of milliseconds since startup

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
  for (int i = 0; i < 14; i++) {
    set_motor_speed(i, 0);
    motor_update(i);
  }

  arm_init();
  log_message(LOG_INFO, "Initialization done\n");
  return 0;
}

int isr(int signum) {
  handle_watchdog();
  for (int i = 0; i < 14; i++) {
    motor_update(i);
  }
#if defined(DEBUG_WRIST) || defined(DEBUG_ELBOW) || defined(DEBUG_BASE) ||     \
    defined(DEBUG_CLAW)
  arm_handle_state_debug();
#else
  arm_handle_state();
#endif

  millis++;
  return 0;
}
