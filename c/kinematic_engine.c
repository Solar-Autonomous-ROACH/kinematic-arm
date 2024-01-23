
#include "kinematic_engine.h"
#include "logger.h"

double law_of_cosines(double a, double b, double c) {
  return acos((b * b + c * c - a * a) / (2 * b * c));
}

int to_deg(double radians) { return (int)(radians * RAD_TO_DEG); }

void kinematic_engine(float x_pos, float y_pos, float z_pos, kinematic_output_t *output) {
  // printf("Values: %f, %f, %f\n", x_pos, y_pos, z_pos);
  double magnitude, theta_1, theta_2, shoulder_rad, elbow_rad;
  clock_t start_time, end_time;

  start_time = clock(); //For timing the whole thing

  /*Convert from vision coords to internal coords*/
  x_pos -= VISION_X_OFFSET;
  y_pos -= VISION_Y_OFFSET;
  z_pos -= VISION_Z_OFFSET; 

  if (!((0 - SMALL_DOUBLE) <= z_pos && z_pos <= SMALL_DOUBLE)) {//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    // If Z is something other than 0, get angle and new x distance
    x_pos -= ROACH_X_OFFSET;
    z_pos -= ROACH_Z_OFFSET;
    output->turn_angle = to_deg(atan(z_pos / x_pos));
    return
  } else {
    output->turn_angle = 0;
  }

  // y_pos += W_C_LENGTH; //accounting for claw position, as should be same
  y_pos += CLAW_Y; //2D Claw angle coming in to grab
  x_pos += CLAW_X;

  // printf("YPos: %f", y_pos);

  magnitude = sqrt(x_pos * x_pos + y_pos * y_pos);
  // printf("Mag: %f\n", magnitude);
  /*Do Checks to see if position is in range and within bounding lines*/
  
  output->extra_distance = 0;
  output->error = false;
  if (magnitude * .99 > S_E_LENGTH + E_W_LENGTH) { // check if in range
    output->extra_distance = (int) (magnitude - S_E_LENGTH - E_W_LENGTH - DISTANCE_OVERSHOOT);
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

  //add angles of two triangles together
  shoulder_rad = theta_1 + atan(y_pos / x_pos);
  elbow_rad = PI - theta_1 - theta_2;

  //get the angles in respect to the servo directions
  output->shoulder_angle = to_deg(PI - shoulder_rad + SHOULDER_CONST);
  output->elbow_angle = to_deg(elbow_rad);
  output->wrist_angle = to_deg(WRIST_CONST + shoulder_rad + elbow_rad);

  end_time = clock();
  log_message(LOG_INFO, "Engine Elapsed: %.1f ms\n",
              (((double)(end_time - start_time)) * 1000.0 / CLOCKS_PER_SEC));
  return;
}