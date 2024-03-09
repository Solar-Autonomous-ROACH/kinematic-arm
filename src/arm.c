#include "_arm.h"
#include "logger.h"
#include <signal.h>
#include <stdlib.h>

// static struct arm_motor_t arm_motor_array[4];

// static arm_motor_t CLAW_MOTOR; // forget about claw for now
arm_motor_t BASE_MOTOR;
arm_motor_t ELBOW_MOTOR;
arm_motor_t WRIST_MOTOR;

bool input_ready = false;
arm_state_t arm_state = ARM_CALIBRATE;
int16_t base_target_angle = 0;
int16_t elbow_target_angle = 0;
int16_t wrist_target_angle = 0;
int16_t claw_target_angle = 0;
int claw_ready = false;
int cycles_since_claw_calibration = 0;
int consecutive_pickup_failures = 0;
vision_info_t original_vision_info;
vision_info_t moved_vision_info;
vision_status_t vision_result;
static arms_calibrate_state_t arms_calibrate_state = ARM_CALIBRATE_START;
kinematic_output_t kinematic_result;

bool arm_requested = false;
bool vision_verify_requested = false;

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
      log_message(LOG_INFO, "Arm fully calibrated\n\n");
    }
    break;
  case ARM_CALIBRATE_READY:
    break;

  default:
    break;
  }
  return arms_calibrate_state;
}

/** none or only one of these should be true at a time */
// #define DEBUG_WRIST
// #define DEBUG_ELBOW
// #define DEBUG_BASE
// #define DEBUG_CLAW
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
#elif defined DEBUG_CLAW

#endif
  return ARM_CALIBRATE_READY;
}

void arm_handle_state() {
  static unsigned long time_in_state = 0; // ms
  arm_motors_status_t status;
  arm_state_t prev_state = arm_state;

  time_in_state++;

  switch (arm_state) {
  case ARM_CALIBRATE:
    // //Temp stuff for now
    // set_motor_speed(CURMOTOR, 30);
    if (arm_calibrate() == ARM_CALIBRATE_READY) {
      arm_state = claw_recalibrate();
      log_message(LOG_INFO, "going to claw calibrate state\n");
    }
    break;

  case CLAW_CALIBRATE:
    if (claw_handle_state() == CLAW_CHECK_POSITION) {
      arm_state = CAPTURE_VISION_INFO;
      log_message(LOG_INFO, "going to capture vision info state\n");
    }
    break;

  case CAPTURE_VISION_INFO:
    if (arm_requested) {
      // send signal to vision python program
      vision_result = vision_receive_input_isr();
      if (vision_result == VISION_READY_FOR_CAPTURE ||
          vision_result == VISION_SUCCESS ||
          vision_result == VISION_SAMPLE_NOT_FOUND) {
        log_message(LOG_INFO, "Requesting vision coordinates\n");
        vision_request_coordinates();
        arm_state = WAIT_FOR_INPUT;
        arm_requested = false;
      } else {
        log_message(LOG_DEBUG, "Vision not requested, vision result = %d\n",
                    vision_result);
      }
    } else {
      log_message(LOG_DEBUG, "Arm not requested\n");
    }
    break;

  case WAIT_FOR_INPUT:
    // wait for coordinates and orientation info from vision team
    vision_result = vision_receive_input_isr();
    switch (vision_result) {
    case VISION_TERMINATED:
    case VISION_ERROR:
      log_message(LOG_INFO, "Vision terminated or error, re-initializing\n");
      raise(SIGINT); // sucide: not recommended for humans
      break;
    case VISION_IN_PROGRESS:
      // do nothing, just waiting for capture to finish
      break;
    case VISION_SAMPLE_NOT_FOUND:
      arm_state = ROVER_MOVING;
      log_message(LOG_WARNING, "No test tube found: %.3f\n",
                  original_vision_info.confidence);
      // move the rover to some place idk
      // rover_move_x(int64_t dist, double speed)
      // rover_rotate(int dir, int angle)
      break;
    case VISION_SUCCESS:
      log_message(LOG_INFO,
                  "Vision returned success, processing coordinates...\n");
      if (!vision_get_coordinates(&original_vision_info)) {
        perror("read original coordinates error");
        raise(SIGINT);
      }
      if (original_vision_info.confidence < VISION_CONFIDENCE_MINIMUM) {
        arm_state = ROVER_MOVING;
        log_message(LOG_WARNING, "Confidence level %lf is too low\n",
                    original_vision_info.confidence);
        // move the rover to some place idk
        // rover_move_x(int64_t dist, double speed)
        // rover_rotate(int dir, int angle)
      } else {
        kinematic_engine(original_vision_info.x, original_vision_info.y,
                         original_vision_info.z, original_vision_info.angle,
                         &kinematic_result);
        if (kinematic_result.error) {
          log_message(LOG_INFO, "Kinematic Engine error\n");
          arm_state = CAPTURE_VISION_INFO;
          break;
        } else if (!validate_kinematic_result(kinematic_result)) {
          // USE ROVER API TO MOVE - can't reach the object
          arm_state = ROVER_MOVING;
          log_message(LOG_INFO, "Validate kinematic result returned false\n");
          arm_state = CAPTURE_VISION_INFO;
          // rover_move_x(kinematic_result.extra_distance, speed); // moving
          // forward rover_rotate(int dir, kinematic_result.turn_angle); // turn
          // angle is +90 to -90. make sure this is
          //  adjusted to whatever rover team provides
        } else {
          log_message(LOG_INFO,
                      "Got input, Base: %hd, Elbow: %hd, Wrist: %hd, Claw: "
                      "%hd, heading "
                      "to PREPARE "
                      "FOR MOVE\n",
                      base_target_angle, elbow_target_angle, wrist_target_angle,
                      claw_target_angle);
          set_joints_angle(base_target_angle, elbow_target_angle, 0);
          // set_claw_angle(claw_target_angle);
          open_claw();
          if (base_target_angle == 0 && elbow_target_angle == 0 &&
              wrist_target_angle == 0 && claw_target_angle == 0) {
            move_home();
          } else {
            arm_state = MOVE_TARGET_BE1;
          }
        }
      }
      break;
    default:
      // should never be STARTING or READY_FOR_CAPTURE
      log_message(LOG_ERROR,
                  "Vision returned an unexpected status in WAIT_FOR_INPUT\n");
      break;
    }
    break;

  case ROVER_MOVING:
    arm_state = CAPTURE_VISION_INFO;
    // if (check_rover_done()) {
    //   // if (true) {
    //   arm_state = CAPTURE_VISION_INFO;
    // }
    break;

  case MOVE_TARGET_BE1:
    status = arm_motors_state_handler(true, true, false);
    if (status == ARM_MOTORS_ERROR) {
      arm_state = arm_recalibrate();
    } else {
      double elbow_angle = get_motor_angle(&ELBOW_MOTOR);
      if (elbow_angle >= elbow_target_angle / 2 &&
          claw_handle_state() == CLAW_CHECK_POSITION) {
        set_joints_angle(base_target_angle, elbow_target_angle,
                         wrist_target_angle);
        arm_state = MOVE_TARGET_WRIST;
      }
    }
    break;

  case MOVE_TARGET_WRIST:
    status = arm_motors_state_handler(true, true, true);
    if (status == ARM_MOTORS_ERROR) {
      arm_state = arm_recalibrate();
    } else if (status == ARM_MOTORS_READY) {
      log_message(LOG_INFO, "MOVE_TARGET complete, heading to CLAW_ACQUIRE\n");
      arm_state = CLAW_ACQUIRE;
      close_claw();
    }
    break;

  case CLAW_ACQUIRE:
    if (claw_handle_state() == CLAW_CHECK_POSITION) {
      double current_base_angle = get_motor_angle(&BASE_MOTOR);
      set_joint_angle(&BASE_MOTOR, -current_base_angle - 20);
      log_message(LOG_INFO,
                  "CLAW_ACQUIRE complete, heading to CLAW_CHECK and raising "
                  "base by 20 degrees, current base angle: %f\n",
                  current_base_angle);
      arm_state = CLAW_CHECK;
    }

    break;

  case CLAW_CHECK:
    if (time_in_state < 1000) {
      return;
    }
    if (vision_verify_requested) {
      vision_verify_requested = false;
      vision_result = vision_receive_input_isr();
      status = arm_motors_state_handler(true, false, false);
      if (status == ARM_MOTORS_ERROR) {
        arm_state = arm_recalibrate();
      } else if (status == ARM_MOTORS_READY &&
                 (vision_result == VISION_SUCCESS ||
                  vision_result == VISION_SAMPLE_NOT_FOUND)) {
        if (vision_result == VISION_SUCCESS &&
            !vision_get_coordinates(&moved_vision_info)) {
          log_message(LOG_ERROR, "read moved coordinates error\n");
          raise(SIGINT);
          exit(1);
        }
        if (vision_result == VISION_SAMPLE_NOT_FOUND ||
            verify_pickup(original_vision_info, moved_vision_info)) {
          // set_joints_angle(BASE_PLACE_ANGLE, ELBOW_PLACE_ANGLE,
          // WRIST_PLACE_ANGLE);
          log_message(LOG_INFO, "verify pickup succeeded\n");
          set_joints_angle(BASE_PLACE_ANGLE, ELBOW_PLACE_ANGLE, 0);
          arm_state = MOVE_PLACE_1;
          log_message(LOG_INFO,
                      "CLAW_CHECK complete, heading to MOVE_PLACE_1\n");
          set_claw_angle(0);
          consecutive_pickup_failures = 0;
        } else {
          log_message(LOG_INFO, "verify pickup failed\n");
          consecutive_pickup_failures++;
          if (consecutive_pickup_failures == CONSECUTIVE_PICKUP_FAILURE_MAX) {
            arm_state = ROVER_MOVING;
            log_message(LOG_DEBUG, "3 consecutive failures. Moving home\n");
            // move_home();
            set_joints_angle(BASE_PLACE_ANGLE, ELBOW_PLACE_ANGLE, 0);
            arm_state = MOVE_PLACE_1;
            set_claw_angle(0);
            consecutive_pickup_failures = 0;
            // rover_move_x(int64_t dist, double speed)
            // rover_rotate(int dir, int angle)
            // probably move somewhere else since we can't pick up the current
            // tube
          } else {
            log_message(LOG_DEBUG,
                        "Taking another picture after pickup verify failed\n");
            arm_state = CAPTURE_VISION_INFO; // did not correctly acquire -
                                             // restart by taking new picture
          }
        }
      }
    } else {
      vision_request_coordinates();
      vision_verify_requested = true;
    }

    break;

  case MOVE_PLACE_1:
    if (time_in_state < 1000) {
      return;
    }
    if (claw_handle_state() == CLAW_CHECK_POSITION &&
        arm_motors_state_handler(true, true, true) == ARM_MOTORS_READY) {
      set_joints_angle(BASE_PLACE_ANGLE, ELBOW_PLACE_ANGLE, WRIST_PLACE_ANGLE);
      arm_state = MOVE_PLACE_2;
    }
    break;

  case MOVE_PLACE_2:
    if (arm_motors_state_handler(false, false, true) == ARM_MOTORS_READY) {
      arm_state = CLAW_DROPOFF;
      open_claw();
    }
    break;

  case CLAW_DROPOFF:
    if (time_in_state > 500 && claw_handle_state() == CLAW_CHECK_POSITION &&
        time_in_state > 1000) {
      set_joints_angle(BASE_HOME_ANGLE, ELBOW_HOME_ANGLE, WRIST_HOME_ANGLE);
      arm_state = MOVE_HOME;
    }
    break;

  case MOVE_HOME:
    if (arm_motors_state_handler(true, true, true) == ARM_MOTORS_READY) {
      cycles_since_claw_calibration++;
      log_message(LOG_INFO, "Cycles since claw calibration = %d\n",
                  cycles_since_claw_calibration);
      if (cycles_since_claw_calibration == CLAW_CALIBRATION_PERIOD) {
        arm_state = claw_recalibrate();
        cycles_since_claw_calibration = 0;
        log_message(LOG_INFO,
                    "MOVE_HOME complete, heading to CLAW_CALIBRATE\n");
      } else {
        arm_state = CAPTURE_VISION_INFO;
        log_message(LOG_INFO,
                    "MOVE_HOME complete, heading to CAPTURE_VISION_INFO\n");
      }
    }
    break;

  default:
    break;
  }

  if (prev_state != arm_state) {
    time_in_state = 0;
  }
}

bool validate_kinematic_result(kinematic_output_t kinematic_result) {
  log_message(LOG_INFO,
              "Validate kinematic result with extra distance = %d, turn angle "
              "= %d, base angle = %d, elbow angle = %d, wrist angle = %d, "
              "error = %d\n",
              kinematic_result.extra_distance, kinematic_result.turn_angle,
              kinematic_result.base_angle, kinematic_result.elbow_angle,
              kinematic_result.wrist_angle, kinematic_result.error);
  if (kinematic_result.extra_distance == 0 &&
      kinematic_result.turn_angle == 0) {
    int base_angle = kinematic_result.base_angle;
    int elbow_angle = kinematic_result.elbow_angle;
    int wrist_angle = kinematic_result.wrist_angle;
    if (base_angle > 10) {
      // base_angle correction
      base_angle += BASE_CORRECTION_ANGLE;
    }
    if (base_angle < 360 && elbow_angle < 360 && wrist_angle < 360) {
      input_ready = true;
      base_target_angle = base_angle;
      elbow_target_angle = elbow_angle;
      wrist_target_angle = wrist_angle;
      claw_target_angle = kinematic_result.claw_angle;
      return true;
    }
  }
  return false;
}

bool verify_pickup(vision_info_t original_vision_info,
                   vision_info_t moved_vision_info) {
  log_message(
      LOG_INFO,
      "Original coordinates: x = %d y = %d z = %d angle = %d conf = %lf\n",
      original_vision_info.x, original_vision_info.y, original_vision_info.z,
      original_vision_info.angle, original_vision_info.confidence);
  log_message(LOG_INFO,
              "Moved coordinates: x = %d y = %d z = %d angle = %d conf = %lf\n",
              moved_vision_info.x, moved_vision_info.y, moved_vision_info.z,
              moved_vision_info.angle, moved_vision_info.confidence);
  int x_diff = moved_vision_info.x - original_vision_info.x;
  int y_diff = moved_vision_info.y - original_vision_info.y;
  int z_diff = moved_vision_info.z - original_vision_info.z;
  int angle_diff = moved_vision_info.angle - original_vision_info.angle;
  return (abs(x_diff) <= X_VERIFICATION_ERROR &&
          y_diff == VERIFICATION_RAISE_DISTANCE &&
          abs(z_diff) <= Z_VERIFICATION_ERROR &&
          abs(angle_diff) <= ANGLE_VERIFICATION_ERROR &&
          moved_vision_info.confidence >= VISION_CONFIDENCE_MINIMUM);
}

void move_home() {
  double elbow_angle = get_motor_angle(&ELBOW_MOTOR);
  if (elbow_angle < ELBOW_HOME_ANGLE) {
    set_joints_angle(BASE_HOME_ANGLE, elbow_angle, WRIST_HOME_ANGLE);
  } else {
    set_joints_angle(BASE_HOME_ANGLE, ELBOW_HOME_ANGLE, WRIST_HOME_ANGLE);
  }
  set_claw_angle(0);
  open_claw();
  arm_state = MOVE_HOME;
}

arm_state_t arm_recalibrate() {
  arms_calibrate_state = ARM_CALIBRATE_START;
  return ARM_CALIBRATE;
}

arm_state_t claw_recalibrate() {
  claw_goto_calibrate();
  return CLAW_CALIBRATE;
}

void arm_begin_pickup() { arm_requested = true; }

bool arm_pickup_done() {
  return arm_state == CAPTURE_VISION_INFO && arm_requested == false;
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
  case ARM_CALIBRATE:
    if (arm_calibrate_debug() == ARM_CALIBRATE_READY) {
      arm_state = WAIT_FOR_INPUT;
      set_joints_angle(90, 90, 90);
      input_ready = true;
      log_message(LOG_INFO, "Calibrate done, heading to WAIT_FOR_INPUT\n");
    }
    break;

  case WAIT_FOR_INPUT:
    if (input_ready) {
      input_ready = false;
      // set_joints_angle(base_target_angle, elbow_target_angle,
      //                  wrist_target_angle);
      // log_message(LOG_INFO, "Got input, heading to PREPARE FOR MOVE\n");
      arm_state = MOVE_TARGET_BE1;
    }
    break;

  case MOVE_TARGET_BE1:
#ifdef DEBUG_WRIST
    if (arm_motor_handle_state(&WRIST_MOTOR) == ARM_MOTOR_CHECK_POSITION) {
      log_message(LOG_INFO, "MOVE_TARGET complete, Waiting for input\n");
      arm_state = WAIT_FOR_INPUT;
    }
    break;
#elif defined DEBUG_ELBOW
    if (arm_motor_handle_state(&ELBOW_MOTOR) == ARM_MOTOR_CHECK_POSITION) {
      log_message(LOG_INFO, "MOVE_TARGET complete, Waiting for input\n");
      arm_state = WAIT_FOR_INPUT;
    }
    break;
#elif defined DEBUG_BASE
    if (arm_motor_handle_state(&BASE_MOTOR) == ARM_MOTOR_CHECK_POSITION) {
      log_message(LOG_INFO, "MOVE_TARGET complete, Waiting for input\n");
      arm_state = WAIT_FOR_INPUT;
    }
    break;
#endif
  default:
    break;
  }
}

/**
 * @brief Stops all the joints of the arm. used for errors.
 */
void stop_arm() {
  set_motor_speed(WRIST_MOTOR.index, 0);
  set_motor_speed(ELBOW_MOTOR.index, 0);
  set_motor_speed(BASE_MOTOR.index, 0);
}

/**
 * @brief call the state handlers of selected motors
 *
 * @param base handle state for base
 * @param elbow handle state for elbow
 * @param wrist handle state for wrist
 * @return arm_motor_state_t state of motors with following priority.
 *         ARM_MOTOR_ERROR>ARM_MOTOR_MOVING_TO_TARGET>ARM_MOTOR_CHECK_POSITION
 */
arm_motors_status_t arm_motors_state_handler(bool base, bool elbow,
                                             bool wrist) {
  bool moving_to_target = false;
  if (base) {
    arm_motor_state_t base_state = arm_motor_handle_state(&BASE_MOTOR);
    if (base_state == ARM_MOTOR_ERROR) {
      stop_arm();
      return ARM_MOTORS_ERROR;
    } else if (base_state == ARM_MOTOR_MOVING_TO_TARGET) {
      moving_to_target = true;
    }
  }
  if (elbow) {
    arm_motor_state_t elbow_state = arm_motor_handle_state(&ELBOW_MOTOR);
    if (elbow_state == ARM_MOTOR_ERROR) {
      stop_arm();
      return ARM_MOTORS_ERROR;
    } else if (elbow_state == ARM_MOTOR_MOVING_TO_TARGET) {
      moving_to_target = true;
    }
  }
  if (wrist) {
    arm_motor_state_t wrist_state = arm_motor_handle_state(&WRIST_MOTOR);
    if (wrist_state == ARM_MOTOR_ERROR) {
      stop_arm();
      return ARM_MOTORS_ERROR;
    } else if (wrist_state == ARM_MOTOR_MOVING_TO_TARGET) {
      moving_to_target = true;
    }
  }
  return moving_to_target ? ARM_MOTORS_MOVING : ARM_MOTORS_READY;
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

void arm_isr() {
  // stores number of milliseconds since startup
  static unsigned long millis __attribute__((unused));
  motor_update_all();
#if defined(DEBUG_WRIST) || defined(DEBUG_ELBOW) || defined(DEBUG_BASE) ||     \
    defined(DEBUG_CLAW)
  arm_handle_state_debug();
#else
  arm_handle_state();
  // set arm done state here?
#endif

  millis++;
}

void arm_init() {
  WRIST_MOTOR.name = "WRIST";
  WRIST_MOTOR.index = WRIST_MOTOR_IDX;
  WRIST_MOTOR.motor = get_motor(WRIST_MOTOR_IDX);
  WRIST_MOTOR.pos_angle = false;
  WRIST_MOTOR.stopper_pos = 0;
  WRIST_MOTOR.is_calibrated = false;
  WRIST_MOTOR.state = ARM_MOTOR_CALIBRATE_INIT;
  WRIST_MOTOR.gear_ratio = 84.294;
  WRIST_MOTOR.CPR = 12;
  WRIST_MOTOR.calibration_speed = 30;
  WRIST_MOTOR.min_speed = 30;
  WRIST_MOTOR.kp = 0.45;
  WRIST_MOTOR.kd = 3;
  WRIST_MOTOR.ki = 0.02;
  WRIST_MOTOR.integral_threshold = 133;

  ELBOW_MOTOR.name = "ELBOW";
  ELBOW_MOTOR.index = ELBOW_MOTOR_IDX;
  ELBOW_MOTOR.motor = get_motor(ELBOW_MOTOR_IDX);
  ELBOW_MOTOR.pos_angle = true;
  ELBOW_MOTOR.stopper_pos = 0;
  ELBOW_MOTOR.is_calibrated = false;
  ELBOW_MOTOR.state = ARM_MOTOR_CALIBRATE_INIT;
  ELBOW_MOTOR.gear_ratio = 270.349;
  ELBOW_MOTOR.CPR = 12;
  ELBOW_MOTOR.calibration_speed = 40;
  ELBOW_MOTOR.min_speed = 40;
  ELBOW_MOTOR.kp = 0.6;
  ELBOW_MOTOR.kd = 6;
  ELBOW_MOTOR.ki = 0.1;
  ELBOW_MOTOR.integral_threshold = 100;

  BASE_MOTOR.name = "BASE";
  BASE_MOTOR.index = BASE_MOTOR_IDX;
  BASE_MOTOR.motor = get_motor(BASE_MOTOR_IDX);
  BASE_MOTOR.pos_angle = false;
  BASE_MOTOR.stopper_pos = 0;
  BASE_MOTOR.is_calibrated = false;
  BASE_MOTOR.state = ARM_MOTOR_CALIBRATE_INIT;
  BASE_MOTOR.gear_ratio = 61.659 * 22.5;
  BASE_MOTOR.CPR = 12;
  BASE_MOTOR.calibration_speed = 40;
  BASE_MOTOR.min_speed = 30;
  BASE_MOTOR.kp = 1;
  BASE_MOTOR.kd = 1;
  BASE_MOTOR.ki = 0.1;
  BASE_MOTOR.integral_threshold = 1000;

  motor_init_all();
  claw_init();

  isr_attach_function(arm_isr);
}

void arm_close() { motor_close_all(); }
