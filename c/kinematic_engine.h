#ifndef KINEMATIC_ENGINE_H
#define KINEMATIC_ENGINE_H

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define S_E_LENGTH 238
#define E_W_LENGTH 273
#define W_C_LENGTH 140

#define PI 3.1416
#define RAD_TO_DEG (180.0 / PI)
/*This is the 270 deg that is used to finish the triangle
  and the compensator for where the wrist starts measuring from*/
#define WRIST_CONST (280.0 / 180.0 * PI - 3 * PI / 2)
/*This is for any tilts in the robot body itself*/
#define SHOULDER_CONST (0.0 / 180.0 * PI)


#define SMALL_DOUBLE 0.000001

double law_of_cosines(double a, double b, double c);
bool kinematic_engine(float x_pos, float y_pos, float z_pos,
                      int16_t *shoulder_angle, int16_t *elbow_angle,
                      int16_t *wrist, int16_t *turn_angle);
int to_deg(double radians);

#endif