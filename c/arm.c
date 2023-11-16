#include "arm.h"

// static struct arm_motor_t arm_motor_array[4];

// static arm_motor_t CLAW_MOTOR; // forget about claw for now
arm_motor_t BASE_MOTOR;
arm_motor_t ELBOW_MOTOR;
arm_motor_t WRIST_MOTOR;

bool input_ready = false;
arm_state_t arm_state = CALIBRATE;
int16_t base_target_angle = 0;
int16_t elbow_target_angle = 0;
int16_t wrist_target_angle = 0;
static arms_calibrate_state_t arms_calibrate_state = ARM_CALIBRATE_START;

/**
 * @brief State machine which goes through the motors and calibrates them
 * @return arms_calibrate_state_t return the state of arm
 */
arms_calibrate_state_t arm_calibrate() {
  switch (arms_calibrate_state) {
  case ARM_CALIBRATE_START:
    arms_calibrate_state = ARM_CALIBRATE_WRIST;
    WRIST_MOTOR.state = ARM_MOTOR_CALIBRATE_INIT;
    ELBOW_MOTOR.state = ARM_MOTOR_CALIBRATE_INIT;
    BASE_MOTOR.state = ARM_MOTOR_CALIBRATE_INIT;
    break;

  case ARM_CALIBRATE_WRIST:
    if (calibrate_handle_state(&WRIST_MOTOR) == ARM_MOTOR_CHECK_POSITION) {
      arms_calibrate_state = ARM_CALIBRATE_PREPARE_ELBOW;
      printf("Calibrate wrist done\n");
    }
    break;
  case ARM_CALIBRATE_PREPARE_ELBOW:
    if (calibrate_handle_state(&ELBOW_MOTOR) == ARM_MOTOR_CHECK_POSITION) {
      arms_calibrate_state = ARM_CALIBRATE_BASE;
      // assume elbow calibrate was not successful
      ELBOW_MOTOR.state = ARM_MOTOR_CALIBRATE_INIT;
      printf("Calibrate elbow attempt\n");
    }
    break;
  case ARM_CALIBRATE_BASE:
    if (calibrate_handle_state(&BASE_MOTOR) == ARM_MOTOR_CHECK_POSITION) {
      arms_calibrate_state = ARM_CALIBRATE_ELBOW;
      printf("Calibrate base done\n");
    }
    break;
  case ARM_CALIBRATE_ELBOW:
    if (calibrate_handle_state(&ELBOW_MOTOR) == ARM_MOTOR_CHECK_POSITION) {
      arms_calibrate_state = ARM_CALIBRATE_READY;
      printf("Calibrate elbow done\n");
    }
    break;
  case ARM_CALIBRATE_READY:
    break;

  default:
    break;
  }
  return arms_calibrate_state;
}

/** only one of these should be true at a time */
// #define DEBUG_WRIST
// #define DEBUG_ELBOW
// #define DEBUG_BASE
/**
 * @brief State machine which goes through the motors and calibrates them
 * @return arms_calibrate_state_t return the state of arm
 */
arms_calibrate_state_t arm_calibrate_debug() {
#ifdef DEBUG_WRIST
  static arms_calibrate_state_t arms_calibrate_state = ARM_CALIBRATE_WRIST;
  if (calibrate_handle_state(&WRIST_MOTOR) == ARM_MOTOR_CHECK_POSITION) {
    arms_calibrate_state = ARM_CALIBRATE_READY;
  }
  return arms_calibrate_state;
#elif defined DEBUG_ELBOW
  static arms_calibrate_state_t arms_calibrate_state = ARM_CALIBRATE_ELBOW;
  if (calibrate_handle_state(&ELBOW_MOTOR) == ARM_MOTOR_CHECK_POSITION) {
    arms_calibrate_state = ARM_CALIBRATE_READY;
  }
  return arms_calibrate_state;
#elif defined DEBUG_BASE
  static arms_calibrate_state_t arms_calibrate_state = ARM_CALIBRATE_BASE;
  if (calibrate_handle_state(&BASE_MOTOR) == ARM_MOTOR_CHECK_POSITION) {
    arms_calibrate_state = ARM_CALIBRATE_READY;
  }
  return arms_calibrate_state;
#endif
  return ARM_CALIBRATE_READY;
}

// validate the set of angles
// eventually move this to be with the kinematic engine
void validate_angle_set(int16_t base_angle, int16_t elbow_angle,
                        int16_t wrist_angle) {
  // if ((base_angle >= 0 && elbow_angle >= 0 && wrist_angle >= 0) &&
  // add more tests in future
  if (base_angle < 360 && elbow_angle < 360 && wrist_angle < 360) {
    input_ready = true;
    base_target_angle = base_angle;
    elbow_target_angle = elbow_angle;
    wrist_target_angle = wrist_angle;
  }
}

void arm_handle_state() {
  switch (arm_state) {
  case CALIBRATE:
    // //Temp stuff for now
    // set_motor_speed(CURMOTOR, 30);
    if (arm_calibrate() == ARM_CALIBRATE_READY) {
      arm_state = WAIT_FOR_INPUT;
      printf("Calibrate done, heading to WAIT_FOR_INPUT\n");
    }
    break;

  case WAIT_FOR_INPUT:
    // wait for coordinates and orientation info from vision team
    if (input_ready) {
      input_ready = false;
      printf("Got input, Base: %hd, Elbow: %hd, Wrist: %hd, heading to PREPARE "
             "FOR MOVE\n",
             base_target_angle, elbow_target_angle, wrist_target_angle);
      set_joints_angle(base_target_angle, elbow_target_angle, 0);
      if (base_target_angle == 0 && elbow_target_angle == 0 &&
          wrist_target_angle == 0) {
        // get elbow angle
        // if (elbow angle < ELBOW_HOME_ANGLE_1)
        // set joint_angle (0, 0,0)
        // goto MOVE_HOME_2
        // else
        // set join angle (0, 45, 0)
        // go to MOVE_HOME_1
        if (get_motor_angle(&ELBOW_MOTOR) < ELBOW_HOME_ANGLE_1) {
          set_joints_angle(BASE_HOME_ANGLE, ELBOW_HOME_ANGLE_2,
                           WRIST_HOME_ANGLE);
          arm_state = MOVE_HOME_2;
        } else {
          set_joints_angle(BASE_HOME_ANGLE, ELBOW_HOME_ANGLE_1,
                           WRIST_HOME_ANGLE);
          arm_state = MOVE_HOME_1;
        }
      } else {
        arm_state = MOVE_TARGET_BE1;
      }
    }
    break;

  case MOVE_TARGET_BE1:
    arm_motor_handle_state(&BASE_MOTOR);
    arm_motor_handle_state(&ELBOW_MOTOR);
    double elbow_angle = get_motor_angle(&ELBOW_MOTOR);
    if (elbow_angle >= elbow_target_angle / 2) {
      set_joints_angle(base_target_angle, elbow_target_angle,
                       wrist_target_angle);
      printf("MOVE_TARGET complete, heading to MOVE_HOME\n");
      arm_state = MOVE_TARGET_WRIST;
    }
    break;

  case MOVE_TARGET_WRIST:
    if (arm_movement_complete()) {
      // set_joints_angle(base_target_angle, elbow_target_angle,
      // wrist_target_angle);
      printf("MOVE_TARGET complete, heading to WAIT_FOR_INPUT\n");
      arm_state = WAIT_FOR_INPUT;
    }
    break;

  case CLAW_ACQUIRE:
    // grab the object
    arm_state = PLACE_TARGET;
    break;

  case PLACE_TARGET:
    // motor angles for placing object will be constant so just move to those
    // angles open the claw?
    set_joints_angle(BASE_PLACE_ANGLE, ELBOW_PLACE_ANGLE, WRIST_PLACE_ANGLE);
    if (arm_movement_complete()) {
      arm_state = MOVE_HOME;
    }

    break;

  case MOVE_HOME_1:
    if (arm_movement_complete()) {
      set_joint_angle(&ELBOW_MOTOR, ELBOW_HOME_ANGLE_2);
      arm_state = MOVE_HOME_2;
    }
    break;

  case MOVE_HOME_2:
    if (arm_movement_complete()) {
      arm_state = WAIT_FOR_INPUT;
      printf("MOVE_HOME complete, heading to WAIT_FOR_INPUT\n");
    }
    break;

  default:
    break;
  }
}

/**
 * @brief Set the joints angle
 *
 * @param base_angle if -1, don't modify angle value
 * @param elbow_angle if -1, don't modify angle value
 * @param elbow_angle if -1, don't modify angle value
 */
void set_joints_angle(int16_t base_angle, int16_t elbow_angle,
                      int16_t wrist_angle) {
  if (base_angle >= 0) {
    set_joint_angle(&BASE_MOTOR, base_angle);
  }
  if (elbow_angle >= 0) {
    set_joint_angle(&ELBOW_MOTOR, elbow_angle);
  }
  if (wrist_angle >= 0) {
    set_joint_angle(&WRIST_MOTOR, wrist_angle);
  }
}

void arm_handle_state_debug() {
  switch (arm_state) {
  case CALIBRATE:
    if (arm_calibrate_debug() == ARM_CALIBRATE_READY) {
      arm_state = WAIT_FOR_INPUT;
      printf("Calibrate done, heading to WAIT_FOR_INPUT\n");
    }
    break;

  case WAIT_FOR_INPUT:
    if (input_ready) {
      input_ready = false;
      set_joints_angle(base_target_angle, elbow_target_angle,
                       wrist_target_angle);
      printf("Got input, heading to PREPARE FOR MOVE\n");
      arm_state = MOVE_TARGET_BE1;
    }
    break;

  case MOVE_TARGET_BE1:
#ifdef DEBUG_WRIST
    if (arm_motor_handle_state(&WRIST_MOTOR) == ARM_MOTOR_CHECK_POSITION) {
      printf("MOVE_TARGET complete, Waiting for input\n");
      arm_state = WAIT_FOR_INPUT;
    }
    break;
#elif defined DEBUG_ELBOW
    if (arm_motor_handle_state(&ELBOW_MOTOR) == ARM_MOTOR_CHECK_POSITION) {
      printf("MOVE_TARGET complete, Waiting for input\n");
      arm_state = WAIT_FOR_INPUT;
    }
    break;
#elif defined DEBUG_BASE
    if (arm_motor_handle_state(&BASE_MOTOR) == ARM_MOTOR_CHECK_POSITION) {
      printf("MOVE_TARGET complete, Waiting for input\n");
      arm_state = WAIT_FOR_INPUT;
    }
    break;
#endif
  default:
    break;
  }
}

// #define GEAR_RATIO 171.7877
// #define GEAR_RATIO 172
// #define CPR 48

bool arm_movement_complete() {
  arm_motor_state_t base_state = arm_motor_handle_state(&BASE_MOTOR);
  arm_motor_state_t elbow_state = arm_motor_handle_state(&ELBOW_MOTOR);
  arm_motor_state_t wrist_state = arm_motor_handle_state(&WRIST_MOTOR);

  return (base_state == ARM_MOTOR_CHECK_POSITION &&
          elbow_state == ARM_MOTOR_CHECK_POSITION &&
          wrist_state == ARM_MOTOR_CHECK_POSITION);
}

void set_joint_angle(arm_motor_t *arm_motor, uint16_t angle) {
  // printf("in set_joint_angle, angle %d\n", angle);
  long ticks = angle * arm_motor->CPR * arm_motor->gear_ratio / 360;
  if (arm_motor->pos_angle) { // if the motor takes in postive angles use
                              // postive ticks
    // TODO: add a check that low_pos + ticks < high_pos
    // printf("ticks: %ld\n", ticks);
    arm_motor->motor->target_pos = arm_motor->stopper_pos + ticks;
    // arm_motor->motor->target_pos = ticks;
  } else { // if the motor takes in negative angles use negative ticks
    // TODO: add a check that low_pos + ticks < high_pos
    // printf("ticks: %ld\n", ticks);
    arm_motor->motor->target_pos = arm_motor->stopper_pos - ticks;
  }
}

// CURRENTLY
// WRIST_MOTOR_PIN 0
// ELBOW_MOTOR_PIN 1
// BASE_MOTOR_PIN 2
// CLAW_MOTOR_PIN 3
void arm_init() {
  WRIST_MOTOR.index = WRIST_MOTOR_PIN;
  WRIST_MOTOR.motor = get_motor(WRIST_MOTOR_PIN);
  WRIST_MOTOR.pos_angle = false;
  WRIST_MOTOR.stopper_pos = 0;
  WRIST_MOTOR.is_calibrated = false;
  WRIST_MOTOR.move_bits = 0xFFFF; // default to all 1s=>assume arm was moving
  WRIST_MOTOR.state = ARM_MOTOR_CALIBRATE_INIT;
  WRIST_MOTOR.gear_ratio = 84.294;
  WRIST_MOTOR.CPR = 12;
  WRIST_MOTOR.calibration_speed = 30;
  WRIST_MOTOR.min_speed = 30;
  WRIST_MOTOR.kp = 0.45;
  WRIST_MOTOR.kd = 3;
  WRIST_MOTOR.ki = 0.02;
  WRIST_MOTOR.integral_threshold = 133;

  ELBOW_MOTOR.index = ELBOW_MOTOR_PIN;
  ELBOW_MOTOR.motor = get_motor(ELBOW_MOTOR_PIN);
  ELBOW_MOTOR.pos_angle = true;
  ELBOW_MOTOR.stopper_pos = 0;
  ELBOW_MOTOR.is_calibrated = false;
  ELBOW_MOTOR.move_bits = 0xFFFF; // default to all 1s=>assume arm was moving
  ELBOW_MOTOR.state = ARM_MOTOR_CALIBRATE_INIT;
  ELBOW_MOTOR.gear_ratio = 270.349;
  ELBOW_MOTOR.CPR = 12;
  ELBOW_MOTOR.calibration_speed = 40;
  ELBOW_MOTOR.min_speed = 40;
  ELBOW_MOTOR.kp = 0.6;
  ELBOW_MOTOR.kd = 6;
  ELBOW_MOTOR.ki = 0.1;
  ELBOW_MOTOR.integral_threshold = 100;

  BASE_MOTOR.index = BASE_MOTOR_PIN;
  BASE_MOTOR.motor = get_motor(BASE_MOTOR_PIN);
  BASE_MOTOR.pos_angle = false;
  BASE_MOTOR.stopper_pos = 0;
  BASE_MOTOR.is_calibrated = false;
  BASE_MOTOR.move_bits = 0xFFFF; // default to all 1s=>assume arm was moving
  BASE_MOTOR.state = ARM_MOTOR_CALIBRATE_INIT;
  BASE_MOTOR.gear_ratio = 61.659 * 22;
  BASE_MOTOR.CPR = 12;
  BASE_MOTOR.calibration_speed = 40;
  BASE_MOTOR.min_speed = 30;
  BASE_MOTOR.kp = 0.05;
  BASE_MOTOR.kd = 0;
  BASE_MOTOR.ki = 0.1;
  BASE_MOTOR.integral_threshold = 1000;

  // CLAW_MOTOR.index = 0;
  // CLAW_MOTOR.motor = get_motor(0); // TODO: Change to correct motor value
  // CLAW_MOTOR.pos_angle = 0;
  // CLAW_MOTOR.stopper_pos = 0;
  // CLAW_MOTOR.is_calibrated = false; // TODO: set me back
  // CLAW_MOTOR.move_bits = 0xFFFF;    // default to all 1s=>assume arm was
  // moving CLAW_MOTOR.state = ARM_MOTOR_CALIBRATE_INIT; // TODO: set me back

  // steer_FR.index = BASE;
  // steer_FR.state = STATE_INITIALIZE;

  // steer_RR.index = ELBOW;
  // steer_RR.state = STATE_INITIALIZE;

  // steer_FL.index = WRIST;
  // steer_FL.state = STATE_INITIALIZE;

  // steer_RL.index = CLAW;
  // steer_RL.state = STATE_INITIALIZE;

  // steering_motor_handle_state(&steer_BASE);
  // steering_motor_handle_state(&steer_ELBOW);
  // steering_motor_handle_state(&steer_WRIST);
  // steering_motor_handle_state(&steer_CLAW);

  // arm_state = ARM_CALIBRATE_WAITING;
}

// int arm_is_calibrated() {
//      return arm_state == ARM_CALIBRATE_READY;
// }

// void arm_init() { //Init function to set things up in isr
//     int i;
//     for (i=0; i<4; i++) {
//         (arm_motor_array[i]).motor = arm_motor_subarray[i];
//     }
// }

// // void rover_update_steering(){
// //     steering_motor_handle_state(&steer_FR);
// //     steering_motor_handle_state(&steer_RR);
// //     steering_motor_handle_state(&steer_FL);
// //     steering_motor_handle_state(&steer_RL);
// // }

// // void rover_stop(){
// //     set_motor_speed(FRW, 0);
// //     set_motor_speed(RRW, 0);
// //     set_motor_speed(FLW, 0);
// //     set_motor_speed(RLW, 0);
// //     set_motor_speed(MRW, 0);
// //     set_motor_speed(MLW, 0);
// // }

// // void rover_forward(int speed){
// //     set_motor_speed(FRW, -speed);
// //     set_motor_speed(MRW, -speed);
// //     set_motor_speed(RRW, -speed);
// //     set_motor_speed(FLW, speed);
// //     set_motor_speed(MLW, speed);
// //     set_motor_speed(RLW, speed);
// // }

// // void rover_reverse(int speed){
// //     set_motor_speed(FRW, speed);
// //     set_motor_speed(MRW, speed);
// //     set_motor_speed(RRW, speed);
// //     set_motor_speed(FLW, -speed);
// //     set_motor_speed(MLW, -speed);
// //     set_motor_speed(RLW, -speed);
// // }

// // void rover_pointTurn_CW(int speed){
// //     set_motor_speed(FRW, speed);
// //     set_motor_speed(MRW, speed);
// //     set_motor_speed(RRW, speed);
// //     set_motor_speed(FLW, speed);
// //     set_motor_speed(MLW, speed);
// //     set_motor_speed(RLW, speed);
// // }

// // void rover_pointTurn_CCW(int speed){
// //     set_motor_speed(FRW, -speed);
// //     set_motor_speed(MRW, -speed);
// //     set_motor_speed(RRW, -speed);
// //     set_motor_speed(FLW, -speed);
// //     set_motor_speed(MLW, -speed);
// //     set_motor_speed(RLW, -speed);
// // }

// // void rover_steer_forward(){
// //     steer_FR.target = steer_FR.center_pos + 0;
// //     steer_FL.target = steer_FL.center_pos + 0;
// //     steer_RR.target = steer_RR.center_pos + 0;
// //     steer_RL.target = steer_RL.center_pos + 0;
// // }

// // void rover_steer_right(int angle){
// //     if(angle >  MAX_STEERING_TICKS) angle =    MAX_STEERING_TICKS;
// //     if(angle < -MAX_STEERING_TICKS) angle =   -MAX_STEERING_TICKS;
// //     steer_FR.target = steer_FR.center_pos + angle;
// //     steer_RR.target = steer_RR.center_pos - angle;
// //     steer_FL.target = steer_FL.center_pos + angle;
// //     steer_RL.target = steer_RL.center_pos - angle;
// // }

// // void rover_steer_left(int angle){
// //     if(angle >  MAX_STEERING_TICKS) angle =  MAX_STEERING_TICKS;
// //     if(angle < -MAX_STEERING_TICKS) angle = -MAX_STEERING_TICKS;
// //     steer_FR.target = steer_FR.center_pos -  angle;
// //     steer_RR.target = steer_RR.center_pos +  angle;
// //     steer_FL.target = steer_FL.center_pos -  angle;
// //     steer_RL.target = steer_RL.center_pos +  angle;
// // }

// // void rover_steer_point(){
// //     steer_FR.target = steer_FR.center_pos - MAX_STEERING_TICKS;
// //     steer_RR.target = steer_RR.center_pos + MAX_STEERING_TICKS;
// //     steer_FL.target = steer_FL.center_pos + MAX_STEERING_TICKS;
// //     steer_RL.target = steer_RL.center_pos - MAX_STEERING_TICKS;
// // }
