#include "kria_arm_motor_controller.h" // Include the header file "motor.h"

static motor_t motors[MAX_MOTORS];

void motor_init() {
  MotorController_init(&(motors[WRIST_MOTOR_IDX].motor_controller),
                       WRIST_MOTOR_ADDRESS);
  MotorController_init(&(motors[ELBOW_MOTOR_IDX].motor_controller),
                       ELBOW_MOTOR_ADDRESS);
  MotorController_init(&(motors[BASE_MOTOR_IDX]).motor_controller,
                       BASE_MOTOR_ADDRESS);
  MotorController_init(&(motors[CLAW_MOTOR_IDX]).motor_controller,
                       CLAW_MOTOR_ADDRESS);
  motor_update_all();
}

void motor_update_all() {
  for (int i = 0; i < MAX_MOTORS; i++) {
    motor_update(i);
  }
}

// This function updates the position of a motor with the given index.
int motor_update(uint8_t motor_index) {
  motor_t *motor = &motors[motor_index];
  uint16_t last;  // Variable to store the last position of the motor
  long last_long; // Variable to store the last position of the motor as a long
                  // integer

  // Check if the motor index is out of range
  if (motor_index >= MAX_MOTORS) {
    fprintf(stderr, "Motor Index Out Of Range"); // Print an error message
    return -1;                                   // Return an error code
  }

  last = motor->raw_pos; // Save the last position of the motor
  last_long = motor->abs_pos;

  // Read the current position of the motor from the MMIO
  // now 16 bits
  MotorController_read(&(motor->motor_controller));
  motor->raw_pos = motor->motor_controller.counts;

  int32_t diff = (int32_t)motor->raw_pos - (int32_t)last;
  // Check if the motor has crossed a threshold in either direction
  if (diff > ENCODER_RESOLUTION_TICKS / 2) {
    motor->abs_pos += ENCODER_RESOLUTION_TICKS - diff;
  } else if (diff < -ENCODER_RESOLUTION_TICKS / 2) {
    // Decrement the absolute position of the motor
    motor->abs_pos -= (ENCODER_RESOLUTION_TICKS + diff);
  } else {
    motor->abs_pos += diff;
  }

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
  motor_t *motor = &motors[motor_index];
  if (speed > 127) {
    speed = 127;
  } else if (speed < -127) {
    speed = -127;
  }
  // convert sign of speed to direction
  motor->motor_controller.dir = speed < 0;
  // map from 0-127 to 0-255
  motor->motor_controller.duty_cycle = abs(speed) << 1;
  MotorController_write(&motor->motor_controller);
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
