#ifndef KINEMATIC_ENGINE_H
#define KINEMATIC_ENGINE_H

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

/*Constants for Arm Length and Approach*/
#define S_E_LENGTH 238
#define E_W_LENGTH 273
#define W_C_LENGTH 140
/*Currently set to be 70 deg*/
#define CLAW_X 48
#define CLAW_Y 132

/*Constants for Conversions*/
/*Vision is offset to where camera is mounted*/
#ifdef VISION_DUMMY
#define VISION_X_OFFSET -20 //-50
#define VISION_Y_OFFSET +0  // 300
#define VISION_Z_OFFSET 0   // 100
#else
#define VISION_X_OFFSET -110 //-50
#define VISION_Y_OFFSET +395 // 300
#define VISION_Z_OFFSET 0    // 100
#endif
/*Roach is offset to center of rover*/
#define ROACH_X_OFFSET -215
#define ROACH_Z_OFFSET -20

/*Constants for Vaild Pickup Area / Boundaries*/
#define LOWER_AREA_BOUND -420
#define BACK_AREA_BOUND 20
#define UPPER_AREA_BOUND 100
#define DISTANCE_OVERSHOOT 50 // How far we overshoot when we're too far away

/*Constants for Math /Kine Stuff*/
#define PI 3.1416
#define RAD_TO_DEG (180.0 / PI)
/*This is the 270 deg that is used to finish the triangle
  and the compensator for where the wrist starts measuring from*/
#define WRIST_CONST ((280.0 + 20.0) / 180.0 * PI - 3 * PI / 2)
/*This is for any tilts in the robot body itself*/
#define SHOULDER_CONST (0.0 / 180.0 * PI)
#define CLAW_CONST 90

#define SMALL_DOUBLE 0.000001

typedef struct {
  int16_t
      extra_distance; // Extra distance to move forward to get within arm range
  int16_t turn_angle; // Angle to turn to align for arm
  int16_t base_angle; // If both 0, fill these out
  int16_t elbow_angle;
  int16_t wrist_angle;
  int16_t claw_angle;
  bool error; // Any other error that causes the engine to have to exit,
              // probably bounds
} kinematic_output_t;

double law_of_cosines(double a, double b, double c);
void kinematic_engine(float x_pos, float y_pos, float z_pos, float angle_pos,
                      kinematic_output_t *output);
// bool kinematic_engine(float x_pos, float y_pos, float z_pos,
//                       int16_t *shoulder_angle, int16_t *elbow_angle,
//                       int16_t *wrist_angle, int16_t *turn_angle);
int to_deg(double radians);

#endif