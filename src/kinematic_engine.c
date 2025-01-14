
#include "kinematic_engine.h"
#include "logger.h"

#include <math.h>
// dummy
double law_of_cosines(double a, double b, double c) {
  return acos((b * b + c * c - a * a) / (2 * b * c));
}

int to_deg(double radians) { return (int)(radians * RAD_TO_DEG); }

void kinematic_engine(float x_pos, float y_pos, float z_pos, float angle_pos,
                      kinematic_output_t *output) {
  // printf("Values: %f, %f, %f\n", x_pos, y_pos, z_pos);
  double magnitude, theta_1, theta_2, shoulder_rad, elbow_rad;
  clock_t start_time, end_time;

  start_time = clock(); // For timing the whole thing

  /*Set output to 0 so no stale values*/
  output->turn_angle = 0;
  output->extra_distance = 0;
  output->error = false;
  output->base_angle = 0;
  output->elbow_angle = 0;
  output->wrist_angle = 0;
  output->claw_angle = 0;

// Vision Plane Compensation
#ifndef VISION_DUMMY
  // y_pos -= VISION_COMP_1_B + (VISION_COMP_1_M * x_pos);
  x_pos = VISION_COMP_2_B + (VISION_COMP_2_M * x_pos);
  // log_message(LOG_INFO, "X_pos = %.3f, Y_pos = %.3f\n", x_pos, y_pos);
#endif

  /*Convert from vision coords to internal coords*/
  x_pos -= VISION_X_OFFSET;
  // y_pos -= VISION_Y_OFFSET;
  y_pos = 0 - VISION_Y_OFFSET;
  z_pos -= VISION_Z_OFFSET;

  log_message(LOG_INFO, "After Offset: X = %.3f, Y = %.3f, Z = %.3f\n", x_pos,
              y_pos, z_pos);

  // Makes sure that tube is within the Z grabby length of the claw
  if (0 - TUBE_CENTER_OFFSET <= z_pos && z_pos <= TUBE_CENTER_OFFSET) {
    output->turn_angle = 0;
  } else {
    x_pos -= ROACH_X_OFFSET;
    z_pos -= ROACH_Z_OFFSET;
    output->turn_angle = to_deg(atan(z_pos / x_pos));
  }

  // y_pos += W_C_LENGTH; //accounting for claw position, as should be same
  y_pos += CLAW_Y; // 2D Claw angle coming in to grab
  x_pos += CLAW_X;

  // Finding Best Claw Attack Angle
  // claw_attack_rad = PI / 2 - atan(x_pos / (0 - y_pos));
  // claw_y = cos(claw_attack_rad) * W_C_LENGTH;
  // claw_x = sin(claw_attack_rad) * W_C_LENGTH;
  // y_pos += claw_y;
  // x_pos += claw_x;
  // log_message(LOG_INFO, "Claw Rad=%.3f, Y_Len=%.3f, X_Len=%.3f\n",
  //             claw_attack_rad, claw_y, claw_x);
  // printf("YPos: %f", y_pos);

  magnitude = sqrt(x_pos * x_pos + y_pos * y_pos);
  // printf("Mag: %f\n", magnitude);
  /*Do Checks to see if position is in range and within bounding lines*/

  // log_message(LOG_INFO, "Testing1\n");
  // log_message(LOG_INFO, "Magnitude = %.3f\n", magnitude);
  // log_message(LOG_INFO, "Arm Length = %.3f\n",
  // (S_E_LENGTH + E_W_LENGTH + 0.0) * 0.98);

  // if (magnitude > (S_E_LENGTH + E_W_LENGTH + 0.0) * 0.98) { // check if in
  // range
  //   // DISTANCE_OVERSHOOT); log_message(LOG_INFO, "Testing2\n");

  if (250 < (x_pos - CLAW_X)) { // demo version for far bound
    // log_message(LOG_INFO, "Magnitude=%.3f\n", magnitude);
    // output->extra_distance =
    //     ((int)(magnitude - S_E_LENGTH - E_W_LENGTH + DISTANCE_OVERSHOOT));
    output->extra_distance = ((int)(x_pos - CLAW_X - 250 + DISTANCE_OVERSHOOT));
    if (output->extra_distance > 1000) {
      output->extra_distance = 100;
    }
    return;
  } else if (LOWER_AREA_BOUND > (y_pos - CLAW_Y)) { // Lower Bound
    output->error = true;
    return;
  } else if (BACK_AREA_BOUND > (x_pos - CLAW_X)) { // Back Bound
    output->error = true;
    return;
  } else if (UPPER_AREA_BOUND < (y_pos - CLAW_Y)) { // Upper Bound
    output->error = true;
    return;
  }

  // Get the angles from the SEW triangle
  theta_1 = law_of_cosines(E_W_LENGTH, magnitude, S_E_LENGTH);
  theta_2 = law_of_cosines(S_E_LENGTH, magnitude, E_W_LENGTH);
  // printf("Thetas: %d, %d", to_deg(theta_1), to_deg(theta_2));

  // add angles of two triangles together
  shoulder_rad = theta_1 + atan(y_pos / x_pos);
  elbow_rad = PI - theta_1 - theta_2;

  // get the angles in respect to the servo directions
  output->base_angle = to_deg(PI - shoulder_rad + SHOULDER_CONST);
  output->elbow_angle = to_deg(elbow_rad);
  output->wrist_angle = to_deg(WRIST_CONST + shoulder_rad + elbow_rad);
  output->claw_angle = (int)angle_pos + CLAW_CONST;

  end_time = clock();
  log_message(LOG_INFO, "Engine Elapsed: %.1f ms\n",
              (((double)(end_time - start_time)) * 1000.0 / CLOCKS_PER_SEC));
  return;
}