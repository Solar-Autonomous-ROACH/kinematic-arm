#include "isr.h"
#include "arm.h"
#include "arm_motor.h"
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
  printf("isr init\n");
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
  printf("HERE\n");
  for (int i = 0; i < 14; i++) {
    set_motor_speed(i, 0);
    motor_update(i);
  }
  // set_target_position(0, get_motor_position(0) - 8245.8096 * 1);
  // printf("Start position %ld, Target position: %ld\n", get_motor_position(0),
  //        get_target_position(0));

  // rover_init();
  printf("ARM INIT\n");
  arm_init();
  printf("ARM INIT DONE\n");
  return 0;
}

// extern arm_motor_t WRIST_MOTOR;
int isr(int signum) {
  // printf("ISR PARTY\n");

  set_PL_register(WATCHDOG_REG, watchdog_flag);
  // set_PL_register(DEBUG_REG, 0xFF);
  for (int i = 0; i < 14; i++) {
    motor_update(i);
  }
  arm_handle_state();
  // printf("Velocity: %5ld\n------------------------\n", velocity);

  // switch (temp >> 10) {
  //     case 0:
  //         if (temp == 1) {printf("Bound 1 = %d\n",
  //         get_PL_register(MOTOR_READ_REG + 0));}
  //         // set_motor_speed(0, ((temp >> 3) & 127));
  //         set_motor_speed(0, 8);
  //         break;
  //     case 1:
  //         // set_motor_speed(0, 128 - ((temp >> 3) & 127));
  //         break;
  //     case 2:
  //         if (temp == 2049) {printf("Bound 2 = %d\n",
  //         get_PL_register(MOTOR_READ_REG + 0));}
  //         // set_motor_speed(0, 0 - ((temp >> 3) & 127));
  //         set_motor_speed(0, -8);
  //         break;
  //     case 3:
  //         // set_motor_speed(0, ((temp >> 3) & 127) - 128);
  //         break;
  //     default:
  //         temp = 0;
  //         // set_motor_speed(0, 50);
  //         break;
  // }
  // temp++;
  // motor_update(0);
  // if (!(temp % 250)){
  //     printf("RAW POSITION = %d\n", get_motor_position(0));
  //     // printf("Temp = %d\n", temp);
  // }

  //     switch (state){
  //         case 10:
  //             rover_init();
  //             // if(enter_distance() == 1){
  //             //     state++;
  //             // }
  //             state++;
  //             break;
  //         case 11:
  //             rover_calibrate();
  //             if(rover_is_calibrated()) state++;
  //             count_ms = 0;
  //             break;
  //         case 12:
  //             rover_steer_point();
  //             if(count_ms >= 1000){
  //                 count_ms = 0;
  //                 state++;
  //             }
  //             break;
  //         case 13:
  //             rover_steer_forward();
  //             if(count_ms >= 1000){
  //                 count_ms = 0;
  //                 state++;
  //             }
  //             break;
  //         case 14:
  //             rover_steer_left(200);
  //             if(count_ms >= 1000){
  //                 count_ms = 0;
  //                 state=12;
  //             }
  //             break;

  //         case 15:
  //             if(enter_distance() == 1){
  //                 state++;
  //             }
  //             break;
  //         default:
  //             //rover_steer_forward();
  // //            rover_move();
  // //            if( finished_moving() == 1){
  // //                state = 15;
  // //                for(int i = 0; i<10; i++){
  // //                    set_motor_speed(i, 0);
  // //                }
  // //            }
  //             //rover_steer_right(200);
  //             // if(count_ms >= 1000){
  //             //     count_ms = 0;
  //             //     state=12;
  //             // }
  //             //new_inc++;
  //             break;
  //     }

  //     if(rover_is_calibrated()){
  //         rover_update_steering();
  //     }

  // set_PL_register(DEBUG_REG, 0x00);
  millis++;
  watchdog_flag = !watchdog_flag;
  count_ms++;
  total_count++;
  return 0;
}
