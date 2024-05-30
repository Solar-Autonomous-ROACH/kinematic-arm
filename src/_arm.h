#ifndef _ARM_H
#define _ARM_H

#include <arm.h>

#include "arm_motor.h"
#include "arm_motor_api.h"
#include "claw.h"
#include "kinematic_engine.h"
#include "simba_bridge.h"
#include "rover.h"
#include "vision.h"
#include <stdint.h>

#define MAX_STEERING_TICKS 300

#define WRIST_PREP_ANGLE 0

#define BASE_HOME_ANGLE 0
#define ELBOW_HOME_ANGLE_1 45
#define ELBOW_HOME_ANGLE_2 0
#define WRIST_HOME_ANGLE 0

#define BASE_PLACE_ANGLE 35
#define ELBOW_PLACE_ANGLE 65
#define WRIST_PLACE_ANGLE 210

// all in millimeters, for pickup verification
#define VERIFICATION_RAISE_DISTANCE 20
#define VERIFICATION_RAISE_MINIMUM 50
#define X_VERIFICATION_ERROR 5
#define Y_VERIFICATION_ERROR 20
#define Z_VERIFICATION_ERROR 0
#define ANGLE_VERIFICATION_ERROR 0

#define CONSECUTIVE_PICKUP_FAILURE_MAX 3

#define BASE_CORRECTION_ANGLE 10

/** only one of these should be true at a time */
// #define DEBUG_WRIST
// #define DEBUG_ELBOW
// #define DEBUG_BASE

typedef enum {
  ARM_CALIBRATE,
  CLAW_CALIBRATE,
  CAPTURE_VISION_INFO,
  WAIT_FOR_INPUT,
  ROVER_STEERING,
  ROVER_MOVING,
  MOVE_TARGET_1,
  MOVE_TARGET_2,
  CLAW_ACQUIRE,
  CLAW_CHECK,
  CLAW_RAISING,
  MOVE_PLACE_1,
  MOVE_PLACE_2,
  CLAW_DROPOFF,
  MOVE_HOME_1,
  MOVE_HOME_2
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

void arm_handle_state(void);
void handle_pickup_validation();
void handle_vision_input();
void stop_motors();
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

#endif // ROVERCORE_ROVER_H
