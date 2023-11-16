#ifndef CLAW_H
#define CLAW_H

#include <stdint.h>

#define CLAW_ERROR_MARGIN 5
#define CLAW_SPEED 30

typedef enum {
  IDLE,
  ROTATE,
  CLOSE,
  ACQUIRED
} claw_state_t;

typedef struct {
    claw_state_t state;
    uint8_t index;
    motor_t *motor;

} claw_motor_t;

claw_state_t claw_handle_state(claw_motor_t * c_motor);
void set_claw_angle(uint16_t angle);
bool claw_open(void);
bool claw_close(void);
bool claw_rotation_complete(void);


#endif