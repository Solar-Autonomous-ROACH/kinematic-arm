#include "arm.h"

// static struct arm_motor_t arm_motor_array[4];

// static arm_motor_t CLAW_MOTOR; // forget about claw for now
arm_motor_t BASE_MOTOR;
arm_motor_t ELBOW_MOTOR;
arm_motor_t WRIST_MOTOR;

/**
 * @brief State machine which goes through the motors and calibrates them
 * @return arms_calibrate_state_t return the state of arm
 */
arms_calibrate_state_t arm_calibrate() {
  static arms_calibrate_state_t arms_calibrate_state = ARM_CALIBRATE_WRIST;
  switch (arms_calibrate_state) {
  case ARM_CALIBRATE_WRIST:
    if (calibrate_handle_state(&WRIST_MOTOR) == ARM_MOTOR_CHECK_POSITION) {
      //   arms_calibrate_state = ARM_CALIBRATE_ELBOW;
      arms_calibrate_state = ARM_CALIBRATE_READY;
    }
    break;
  case ARM_CALIBRATE_PREPARE_ELBOW:
    if (calibrate_handle_state(&ELBOW_MOTOR) == ARM_MOTOR_CHECK_POSITION) {
      arms_calibrate_state = ARM_CALIBRATE_BASE;
      // assume elbow calibrate was not successful
      ELBOW_MOTOR.state = ARM_MOTOR_CALIBRATE_INIT;
    }
    break;
  case ARM_CALIBRATE_BASE:
    if (calibrate_handle_state(&BASE_MOTOR) == ARM_MOTOR_CHECK_POSITION) {
      arms_calibrate_state = ARM_CALIBRATE_ELBOW;
    }
    break;
  case ARM_CALIBRATE_ELBOW:
    if (calibrate_handle_state(&ELBOW_MOTOR) == ARM_MOTOR_CHECK_POSITION) {
      arms_calibrate_state = ARM_CALIBRATE_READY;
    }
    break;
  case ARM_CALIBRATE_READY:
    break;

  default:
    break;
  }
  return arms_calibrate_state;
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

// #define GEAR_RATIO 171.7877
// #define GEAR_RATIO 172
// #define CPR 48

bool arm_movement_complete(){
  return 
       (arm_motor_handle_state(&BASE_MOTOR) == ARM_MOTOR_CHECK_POSITION &&
        arm_motor_handle_state(&ELBOW_MOTOR) == ARM_MOTOR_CHECK_POSITION &&
        arm_motor_handle_state(&WRIST_MOTOR) == ARM_MOTOR_CHECK_POSITION);
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
  WRIST_MOTOR.index = 0;
  WRIST_MOTOR.motor =
      get_motor(WRIST_MOTOR_PIN); // TODO: Change to correct motor value
  WRIST_MOTOR.pos_angle = false;
  WRIST_MOTOR.stopper_pos = 0;
  WRIST_MOTOR.is_calibrated = false; // TODO: set me back
  WRIST_MOTOR.move_bits = 0xFFFF;    // default to all 1s=>assume arm was moving
  WRIST_MOTOR.state = ARM_MOTOR_CALIBRATE_INIT; // TODO: set me back
  WRIST_MOTOR.gear_ratio = 84.294;
  WRIST_MOTOR.CPR = 12;

  ELBOW_MOTOR.index = 0;
  ELBOW_MOTOR.motor =
      get_motor(ELBOW_MOTOR_PIN); // TODO: Change to correct motor value
  ELBOW_MOTOR.pos_angle = true;
  ELBOW_MOTOR.stopper_pos = 0;
  ELBOW_MOTOR.is_calibrated = false; // TODO: set me back
  ELBOW_MOTOR.move_bits = 0xFFFF;    // default to all 1s=>assume arm was moving
  ELBOW_MOTOR.state = ARM_MOTOR_CALIBRATE_INIT; // TODO: set me back
  ELBOW_MOTOR.gear_ratio = 84.294;
  ELBOW_MOTOR.CPR = 12;

  BASE_MOTOR.index = 0;
  BASE_MOTOR.motor =
      get_motor(BASE_MOTOR_PIN); // TODO: Change to correct motor value
  BASE_MOTOR.pos_angle = false;
  BASE_MOTOR.stopper_pos = 0;
  BASE_MOTOR.is_calibrated = false; // TODO: set me back
  BASE_MOTOR.move_bits = 0xFFFF;    // default to all 1s=>assume arm was moving
  BASE_MOTOR.state = ARM_MOTOR_CALIBRATE_INIT; // TODO: set me back
  BASE_MOTOR.gear_ratio = 84.294;
  BASE_MOTOR.CPR = 12;

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
