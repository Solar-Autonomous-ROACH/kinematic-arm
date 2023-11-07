#ifndef KINEMATIC_ENGINE_H
#define KINEMATIC_ENGINE_H


#include <math.h>
#include <stdbool.h>

#define S_E_LENGTH 240
#define E_W_LENGTH 260
#define W_C_LENGTH 130

#define PI 3.1416
#define RAD_TO_DEG (180.0 / PI)


double law_of_cosines(double a, double b, double c);
bool kinematic_engine(float x_pos, float y_pos, float z_pos, 
    int * shoulder_angle, int * elbow_angle, int * wrist_angle);
int to_deg(double radians);


#endif