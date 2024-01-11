#include "arm.h"
#include "logger.h"

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
int16_t claw_target_angle = 0;
int claw_ready = false;
vision_info_t * vision_info;
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

// validate the set of angles
// eventually move this to be with the kinematic engine
bool validate_angle_set(int16_t base_angle, int16_t elbow_angle,
                        int16_t wrist_angle, int16_t claw_angle) {
  // if ((base_angle >= 0 && elbow_angle >= 0 && wrist_angle >= 0) &&
  // add more tests in future
  if (base_angle > 10) {
    // base_angle correction
    base_angle += 12;
  }
  if (base_angle < 360 && elbow_angle < 360 && wrist_angle < 360) {
    input_ready = true;
    base_target_angle = base_angle;
    elbow_target_angle = elbow_angle;
    wrist_target_angle = wrist_angle;
    claw_target_angle = claw_angle % 360;
    return true;
  }
  return false;
}

void arm_handle_state() {
  static unsigned long time_in_state = 0; // ms
  arm_motors_status_t status;
  arm_state_t prev_state = arm_state;

  time_in_state++;

  switch (arm_state) {
  case CALIBRATE:
    // //Temp stuff for now
    // set_motor_speed(CURMOTOR, 30);
    if (arm_calibrate() == ARM_CALIBRATE_READY) {
      arm_state = CAPTURE_VISION_INFO;
      printf("Input: ");
    }
    break;
  
  case CAPTURE_VISION_INFO:
    //send signal to vision python program
    vision_request_coordinates();
    arm_state = WAIT_FOR_INPUT;
    break;

  case WAIT_FOR_INPUT:
    // wait for coordinates and orientation info from vision team
    if (vision_get_status() == VISION_SUCCESS) {
      vision_info = vision_get_coordinates();
      kinematic_engine(vision_info.x, vision_info.y, vision_info.z, &base_target_angle, &elbow_target_angle, &wrist_target_angle, &claw_target_angle);
      if (!validate_angle_set(base_target_angle, elbow_target_angle, wrist_target_angle, claw_target_angle)){
        //USE ROVER API TO MOVE 
        arm_state = ROVER_MOVING;
      } else {
        log_message(
          LOG_INFO,
          "Got input, Base: %hd, Elbow: %hd, Wrist: %hd, heading to PREPARE "
          "FOR MOVE\n",
          base_target_angle, elbow_target_angle, wrist_target_angle);
        set_joints_angle(base_target_angle, elbow_target_angle, 0);
        // set_claw_angle(claw_target_angle);
        open_claw();
        if (base_target_angle == 0 && elbow_target_angle == 0 && wrist_target_angle == 0) {
          move_home();
        } else {
          arm_state = MOVE_TARGET_BE1;
        }
      }
      //input_ready = false;
      
    }
    break;

  case ROVER_MOVING:
    if (rover_movement_done()){
      arm_state = CAPTURE_VISION_INFO;
    }
    break;
  
  case MOVE_TARGET_BE1:
    status = arm_motors_state_handler(true, true, false);
    if (status == ARM_MOTORS_ERROR) {
      arm_state = recalibrate();
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
      arm_state = recalibrate();
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
    status = arm_motors_state_handler(true, false, false);
    if (status == ARM_MOTORS_ERROR) {
      arm_state = recalibrate();
    } else if (status == ARM_MOTORS_READY &&
               true) { // REPLACE "true" WITH VISION TEAMS LOCATION CHECK
      // set_joints_angle(BASE_PLACE_ANGLE, ELBOW_PLACE_ANGLE,
      // WRIST_PLACE_ANGLE);
      set_joints_angle(BASE_PLACE_ANGLE, ELBOW_PLACE_ANGLE, 0);
      arm_state = MOVE_PLACE_1;
      log_message(LOG_INFO, "CLAW_CHECK complete, heading to MOVE_PLACE_1\n");
      // set_claw_angle(0);
      open_claw();
    }

    break;

  case MOVE_PLACE_1:
    if (time_in_state < 1000) {
      return;
    }
    if (arm_motors_state_handler(true, true, true) == ARM_MOTORS_READY) {
      set_joints_angle(BASE_PLACE_ANGLE, ELBOW_PLACE_ANGLE, WRIST_PLACE_ANGLE);
      arm_state = MOVE_PLACE_2;
    }
    break;

  case MOVE_PLACE_2:
    if (arm_motors_state_handler(false, false, true) == ARM_MOTORS_READY) {
      arm_state = CLAW_DROPOFF;
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
      arm_state = CAPTURE_VISION_INFO;
      log_message(LOG_INFO,
                  "MOVE_HOME complete, heading to CAPTURE_VISION_INFO\n");
    }
    break;
    // case PLACE_TARGET:
    //   // motor angles for placing object will be constant so just move to
    //   those
    //   // angles open the claw?
    //   if (arm_motors_state_handler(true, true, true) == ARM_MOTORS_READY &&
    //   claw_handle_state(&CLAW_MOTOR) == ROTATE_TARGET) {
    //     arm_state = MOVE_HOME_1;
    //   }

    //   break;

    // case MOVE_HOME_1:
    //   if (arm_motors_state_handler(true, true, true) == ARM_MOTORS_READY) {
    //     set_joint_angle(&ELBOW_MOTOR, ELBOW_HOME_ANGLE_2);
    //     arm_state = MOVE_HOME_2;
    //   }
    //   break;

    // case MOVE_HOME_2:
    //   if (arm_motors_state_handler(true, true, true) == ARM_MOTORS_READY) {
    //     arm_state = WAIT_FOR_INPUT;
    //     log_message(LOG_INFO,
    //                 "MOVE_HOME complete, heading to WAIT_FOR_INPUT\nInput:
    //                 ");
    //   }
    //   break;

  default:
    break;
  }

  if (prev_state != arm_state) {
    time_in_state = 0;
  }
}

void get_vision_information(void){

}

void move_home() {
  double elbow_angle = get_motor_angle(&ELBOW_MOTOR);
  if (elbow_angle < ELBOW_HOME_ANGLE) {
    set_joints_angle(BASE_HOME_ANGLE, elbow_angle, WRIST_HOME_ANGLE);
  } else {
    set_joints_angle(BASE_HOME_ANGLE, ELBOW_HOME_ANGLE, WRIST_HOME_ANGLE);
  }
  // set_claw_angle(&CLAW_MOTOR, 0);
  open_claw();
  arm_state = MOVE_HOME;
}

arm_state_t recalibrate() {
  arms_calibrate_state = ARM_CALIBRATE_START;
  return CALIBRATE;
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

// CURRENTLY
// WRIST_MOTOR_PIN 0
// ELBOW_MOTOR_PIN 1
// BASE_MOTOR_PIN 2
// CLAW_MOTOR_PIN 3
void arm_init() {
  WRIST_MOTOR.name = "WRIST";
  WRIST_MOTOR.index = WRIST_MOTOR_PIN;
  WRIST_MOTOR.motor = get_motor(WRIST_MOTOR_PIN);
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
  ELBOW_MOTOR.index = ELBOW_MOTOR_PIN;
  ELBOW_MOTOR.motor = get_motor(ELBOW_MOTOR_PIN);
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
  BASE_MOTOR.index = BASE_MOTOR_PIN;
  BASE_MOTOR.motor = get_motor(BASE_MOTOR_PIN);
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

  claw_init();
}
