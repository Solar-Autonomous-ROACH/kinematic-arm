#include "isr.h"
#include "arm.h"
#include "arm_motor.h"
#include "logger.h"
#include "mmio.h"
static int count_ms = 0;
static uint64_t total_count = 120000;
static uint8_t watchdog_flag = 0;
static unsigned long millis; // stores number of milliseconds since startup

int new_inc = 4;
int temp = 0;
#define CURMOTOR 0
#define ERROR_MARGIN 5

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

int isr(int signum __attribute__((unused))) {
  set_PL_register(WATCHDOG_REG, watchdog_flag);
  // set_PL_register(DEBUG_REG, 0xFF);
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
  watchdog_flag = !watchdog_flag;
  count_ms++;
  total_count++;
  return 0;
}
