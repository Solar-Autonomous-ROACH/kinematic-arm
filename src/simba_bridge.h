#ifndef SIMBA_BRIDGE
#define SIMBA_BRIDGE

#define OUTER_RADIUS 345
#define INNER_RADIUS 255

#define NUM_MOTORS_2 15


int armada_rover_rotate(int angle, double speed);
long deg_len_ticks(int angle, int radius);

#endif