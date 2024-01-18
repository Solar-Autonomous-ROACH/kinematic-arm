#include "motor.h" // Include the header file "motor.h"
#include "rover.h"

// static long motor_abs_pos[MAX_MOTORS];
// static long long motor_target_pos[MAX_MOTORS];
// static int motor_velocity[MAX_MOTORS];

// static uint8_t motor_raw_pos[MAX_MOTORS];
static motor_t motors[MAX_MOTORS];
// static uint8_t motor_pwm[MAX_MOTORS];

int MotorController_init(MotorController *motor, off_t mmio_address) {
  // Initialize MMIO
  volatile unsigned int *mmio = mmio_init(mmio_address);

  if (!mmio_is_valid(mmio))
    return -1;

  // Initialize motor controller
  motor->mmio = mmio;
  motor->duty_cycle = 0;
  motor->clk_divisor = 4;
  motor->dir = 1;
  motor->en_motor = 1;
  motor->clear_enc = 1;
  motor->en_enc = 1;

  // Clear encoder counter
  MotorController_write(motor);
  motor->clear_enc = 0;
  MotorController_write(motor);

  return 0;
}

void MotorController_close(MotorController *motor) {
  // Disable motor controller
  motor->en_motor = 0;
  motor->en_enc = 0;
  MotorController_write(motor);

  // Release MMIO
  close_mem(motor->mmio);
}

void MotorController_write(MotorController *motor) {
  // Write to MMIO
  *(motor->mmio) = motor->duty_cycle + (motor->clk_divisor << 8) +
                   (motor->dir << 11) + (motor->en_motor << 12) +
                   (motor->clear_enc << 13) + (motor->en_enc << 14);
}

//motor encoder value
void MotorController_read(MotorController *motor) {
  // Read MMIO
  motor->counts = *(motor->mmio + 2);
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

  // Read the current position of the motor from the MMIO
  MotorController_read(&(motor->motor_controller));
  motor->raw_pos = motor->motor_controller.counts;

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
  motor_t *motor = &motors[motor_index];
  if (speed > 127)
    speed = 127;
  if (speed < -127)
    speed = -127;
  if (speed >= 0)
    // set_PL_register(MOTOR_WRITE_REG + motor_index, speed);
    MotorController_set_speed(&(motor->motor_controller), speed);

  else
    MotorController_set_speed(&(motor->motor_controller), 0xFF + speed);
    // set_PL_register(MOTOR_WRITE_REG + motor_index, 0xFF + speed);
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

char get_raw_pos(uint8_t motor_index) {
  if (motor_index >= MAX_MOTORS) {
    fprintf(stderr, "Motor Index Out Of Range");
    return -1;
  }
  return motors[motor_index].raw_pos;
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
