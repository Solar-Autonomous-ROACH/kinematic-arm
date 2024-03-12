#ifndef ARM_H
#define ARM_H

#include <stdbool.h>

void arm_init();
void arm_close();
void arm_stop();
void arm_begin_pickup();
bool arm_pickup_done();

#endif // ROVERCORE_ROVER_H
