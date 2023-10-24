#include "isr.h"
#include "mmio.h"
static int count_ms = 0;
static int state = 10;
static int motor = 0;
static uint64_t total_count = 120000;
static uint8_t watchdog_flag = 0;
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
  set_target_position(0, get_motor_position(0) - 16512);
  printf("Start position %ld, Target position: %ld\n", get_motor_position(0),
         get_target_position(0));

  // rover_init();
  return 0;
}

typedef enum {
  CHECK_POSITION,
  MOVING_TO_TARGET,
  START_CALIBRATE,
  CHECK_SPEED_POS,
  CHECK_SPEED_NEG
} arm_state;

int isr(int signum) {

  set_PL_register(WATCHDOG_REG, watchdog_flag);
  // set_PL_register(DEBUG_REG, 0xFF);
  static arm_state current_state = CHECK_POSITION;
  // static arm_state current_state = START_CALIBRATE;
  static uint16_t loops_in_start_calibrate = 0;
  motor_update(0);
  long current_position = get_motor_position(0);
  long target_position = get_target_position(0);
  int velocity = get_motor_velocity(0);
  long diff = current_position - target_position;
  long speed;

  switch (current_state) {
  case START_CALIBRATE:
    set_motor_speed(0, 10); /** start motors */
    /** only go to next state if motor has been turning for 100ms*/
    if (loops_in_start_calibrate == 5000) {
      current_state = CHECK_SPEED_POS;
      loops_in_start_calibrate = 0;
      printf("Changing state to CHECK_SPEED_POS\n");
    } else {
      loops_in_start_calibrate++;
    }
    break;

  case CHECK_SPEED_POS:
    if (velocity == 0) {
      current_state = CHECK_SPEED_NEG;
      printf("Changing state to CHECK_SPEED_NEG\n");
      set_motor_speed(0, -10);
    }
    break;

  case CHECK_SPEED_NEG:
    if (velocity == 0) {
      current_state = CHECK_POSITION;
      printf("Changing state to CHECK_POSITON\n");
      set_motor_speed(0, 0);
    }
    break;
  case CHECK_POSITION:
    if (diff > ERROR_MARGIN || diff < -ERROR_MARGIN) {
      current_state = MOVING_TO_TARGET;
      printf("Changing state to MOVING_TO_TARGET\n");
    } else {
      set_motor_speed(0, 0);
      speed = 0;
    }
    /* code */
    break;

  case MOVING_TO_TARGET:
    // postive direction
    if (diff > ERROR_MARGIN || diff < -ERROR_MARGIN) {
      if (diff > 0) {
        // printf("Moving +")
        // set_motor_speed(0, -60);
        speed = (target_position - current_position) * 0.05 - velocity;
        if (speed < -80) {
          speed = -80;
        } else if (speed > -10) {
          speed = -10;
        }
        set_motor_speed(0, speed);
      } else { // negative direction
        // printf("Moving -")
        // set_motor_speed(0, 60);
        speed = (target_position - current_position) * 0.05 + velocity;
        ;
        if (speed > 80) {
          speed = 80;
        } else if (speed < 10) {
          speed = 10;
        }
        set_motor_speed(0, speed);
      }
    } else {
      printf("Changing state to CHECK_POSITON\n");
      current_state = CHECK_POSITION;
      set_motor_speed(0, 0);
      speed = 0;
    }
    break;

  default:
    break;
  }
  // printf("Velocity: %5ld\n------------------------\n", velocity);

  if (temp == 500) {
    temp = 0;
    printf("Raw: %3d | Absolute: %5ld | Speed: %4ld| Velocity: "
           "%4ld\n------------------------\n",
           get_raw_pos(0), current_position, speed, velocity);
  } else {
    temp++;
  }

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
  watchdog_flag = !watchdog_flag;
  count_ms++;
  total_count++;
  return 0;
}
