#ifndef ARM_H
#define ARM_H

#include "arm_motor.h"
#include "arm_motor_controller.h"
#include "claw.h"
#include "kinematic_engine.h"
#include "rover.h"
#include "vision.h"
#include <stdint.h>

// #define STEERING_GEAR_RATIO 172
// #define WHEEL_GEAR_RATIO 212

#define MAX_STEERING_TICKS 300

#define WRIST_PREP_ANGLE 0

#define BASE_HOME_ANGLE 0
#define ELBOW_HOME_ANGLE 0
#define WRIST_HOME_ANGLE 0

#define BASE_PLACE_ANGLE 40
#define ELBOW_PLACE_ANGLE 45
#define WRIST_PLACE_ANGLE 200

//all in millimeters, for pickup verification
#define VERIFICATION_RAISE_DISTANCE 100
#define VERIFICATION_RAISE_MINIMUM 50
#define X_VERIFICATION_ERROR 5
#define Y_VERIFICATION_ERROR 20
#define Z_VERIFICATION_ERROR 0
#define ANGLE_VERIFICATION_ERROR 0

#define CONSECUTIVE_PICKUP_FAILURE_MAX 3

/** only one of these should be true at a time */
// #define DEBUG_WRIST
// #define DEBUG_ELBOW
// #define DEBUG_BASE

typedef enum {
  ARM_CALIBRATE,
  CLAW_CALIBRATE,
  CAPTURE_VISION_INFO,
  WAIT_FOR_INPUT,
  ROVER_MOVING,
  MOVE_TARGET_BE1,
  MOVE_TARGET_WRIST,
  CLAW_ACQUIRE,
  CLAW_CHECK,
  MOVE_PLACE_1,
  MOVE_PLACE_2,
  CLAW_DROPOFF,
  MOVE_HOME
} arm_state_t;

typedef enum {
  ARM_CALIBRATE_START,
  ARM_CALIBRATE_WRIST,
  ARM_CALIBRATE_PREPARE_ELBOW,
  ARM_CALIBRATE_BASE,
  ARM_CALIBRATE_ELBOW,
  ARM_CALIBRATE_CLAW,
  ARM_CALIBRATE_READY
} arms_calibrate_state_t;

typedef enum {
  ARM_MOTORS_ERROR,
  ARM_MOTORS_MOVING,
  ARM_MOTORS_READY
} arm_motors_status_t;

// extern static struct arm_motor_t arm_motor_array[4];
// extern static struct motor_t arm_motor_subarray[4];

void arm_init();
void arm_handle_state(void);
void stop_arm();
arm_motors_status_t arm_motors_state_handler(bool base, bool elbow, bool wrist);
void move_home();
void arm_handle_state_debug();

arms_calibrate_state_t arm_calibrate();
arm_state_t arm_recalibrate();
arm_state_t claw_recalibrate();

void set_joints_angle(int16_t base_angle, int16_t elbow_angle,
                      int16_t wrist_angle);
void set_joint_angle(arm_motor_t *arm_motor, uint16_t angle);

bool arm_movement_complete();
bool verify_pickup(vision_info_t original_vision_info,
                   vision_info_t moved_vision_info);
bool validate_kinematic_result(kinematic_output_t kinematic_result);

// extern arm_motor_t BASE_MOTOR;
// extern arm_motor_t ELBOW_MOTOR;
// extern arm_motor_t WRIST_MOTOR;

// void arm_init();

// void rover_stop();

// void arm_forward(int speed);

// void arm_reverse(int speed);

// void arm_pointTurn_CW(int speed);

// void arm_pointTurn_CCW(int speed);

// void arm_steer_forward();

// void arm_steer_right(int angle);

// void arm_steer_left(int angle);

// void arm_steer_point();

#endif // ROVERCORE_ROVER_H
