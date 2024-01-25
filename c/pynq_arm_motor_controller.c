#include "pynq_arm_motor_controller.h" // Include the header file "motor.h"

// static long motor_abs_pos[MAX_MOTORS];
// static long long motor_target_pos[MAX_MOTORS];
// static int motor_velocity[MAX_MOTORS];

// static uint8_t motor_raw_pos[MAX_MOTORS];
static motor_t motors[MAX_MOTORS];
// static uint8_t motor_pwm[MAX_MOTORS];

void motor_init() {
  motors[WRIST_MOTOR_IDX].pin = WRIST_MOTOR_PIN;
  motors[ELBOW_MOTOR_IDX].pin = ELBOW_MOTOR_PIN;
  motors[BASE_MOTOR_IDX].pin = BASE_MOTOR_PIN;
  motors[CLAW_MOTOR_IDX].pin = CLAW_MOTOR_PIN;
}

// This function updates the position of a motor with the given index.
int motor_update(uint8_t motor_index) {
  motor_t *motor = &motors[motor_index];
  uint8_t last;   // Variable to store the last position of the motor
  long last_long; // Variable to store the last position of the motor as a long
                  // integer

  // Check if the motor index is out of range
  if (motor_index >= MAX_MOTORS) {
    fprintf(stderr, "Motor Index Out Of Range"); // Print an error message
    return -1;                                   // Return an error code
  }

  last = motor->raw_pos; // Save the last position of the motor
  last_long = motor->abs_pos;
  // Read the current position of the motor from the I/O register
  motor->raw_pos = get_PL_register(MOTOR_READ_REG + motor->pin);
  // Check if the motor has crossed a threshold in either direction
  if ((last > HIGH_THRESH) && (motor->raw_pos < LOW_THRESH)) {
    motor->abs_pos +=
        1 << MOTOR_DATA_SIZE; // Increment the absolute position of the motor
  }
  if ((last < LOW_THRESH) && (motor->raw_pos > HIGH_THRESH)) {
    motor->abs_pos -=
        1 << MOTOR_DATA_SIZE; // Decrement the absolute position of the motor
  }

  motor->abs_pos &= UPPER_MASK; // Mask the upper bits of the absolute position
  motor->abs_pos += motor->raw_pos; // Add the raw position to the absolute
                                    // position
  set_motor_position(motor_index, motor->abs_pos);

  motor->velocity = motor->abs_pos - last_long;
  if (motor->velocity == 0) {
    motor->stopped_duration = motor->stopped_duration == UINT16_MAX
                                  ? UINT16_MAX
                                  : motor->stopped_duration + 1;
  } else {
    motor->stopped_duration = 0;
  }

  return 0; // Return success code
}

int set_motor_speed(uint8_t motor_index, int speed) {
  if (motor_index >= MAX_MOTORS) {
    fprintf(stderr, "Motor Index Out Of Range"); // Print an error message
    return -1;                                   // Return an error code
  }
  if (speed > 127)
    speed = 127;
  if (speed < -127)
    speed = -127;
  if (speed >= 0)
    set_PL_register(MOTOR_WRITE_REG + motors[motor_index].pin, speed);
  else
    set_PL_register(MOTOR_WRITE_REG + motors[motor_index].pin, 0xFF + speed);
  return 0;
}

long get_motor_position(uint8_t motor_index) {
  if (motor_index >= MAX_MOTORS) {
    fprintf(stderr, "Motor Index Out Of Range");
    return -1;
  }
  return motors[motor_index].abs_pos;
}

void set_motor_position(uint8_t motor_index, long position) {
  if (motor_index >= MAX_MOTORS) {
    fprintf(stderr, "Motor Index Out Of Range");
    return;
  }
  motors[motor_index].position_fifo_idx++;
  if (motors[motor_index].position_fifo_idx >= POSITION_FIFO_SIZE) {
    motors[motor_index].position_fifo_idx -= POSITION_FIFO_SIZE;
  }
  motors[motor_index].position_fifo[motors[motor_index].position_fifo_idx] =
      position;
}

long get_motor_position_n(uint8_t motor_index, uint8_t n) {
  if (motor_index >= MAX_MOTORS) {
    fprintf(stderr, "Motor Index Out Of Range");
    return -1;
  } else if (n >= POSITION_FIFO_SIZE) {
    fprintf(stderr, "Motor FIFO index out of bounds");
    return -1;
  }
  int16_t index = motors[motor_index].position_fifo_idx - n;
  if (index < 0) {
    index += POSITION_FIFO_SIZE;
  }
  return motors[motor_index].position_fifo[index];
}

long long get_target_position(uint8_t motor_index) {
  if (motor_index >= MAX_MOTORS) {
    fprintf(stderr, "Motor Index Out Of Range");
    return -1;
  }
  return motors[motor_index].target_pos;
}

int set_target_position(uint8_t motor_index, long long target_position) {
  if (motor_index >= MAX_MOTORS) {
    fprintf(stderr, "Motor Index Out Of Range");
    return -1;
  }
  motors[motor_index].target_pos = target_position;
  return 0;
}

int get_motor_velocity(uint8_t motor_index) {
  if (motor_index >= MAX_MOTORS) {
    fprintf(stderr, "Motor Index Out Of Range");
    return -1;
  }
  return motors[motor_index].velocity;
}

motor_t *get_motor(uint8_t motor_index) {
  if (motor_index >= MAX_MOTORS) {
    fprintf(stderr, "Motor Index Out Of Range");
    return NULL;
  }
  return &motors[motor_index];
}