
#include "kinematic_engine.h"
#include <stdio.h>

double law_of_cosines(double a, double b, double c) {
    return acos((b * b + c * c - a * a) / (2 * b * c));
}


int to_deg(double radians) {
    return (int) (radians * RAD_TO_DEG);
}


bool kinematic_engine(float x_pos, float y_pos, float z_pos, 
    int16_t * shoulder_angle, int16_t * elbow_angle, int16_t * wrist_angle) {
        // printf("Values: %f, %f, %f\n", x_pos, y_pos, z_pos);
        double magnitude, theta_1, theta_2, shoulder_rad, elbow_rad;

        y_pos += W_C_LENGTH;

        // printf("YPos: %f", y_pos);

        magnitude = sqrt(x_pos * x_pos + y_pos * y_pos);
        // printf("Mag: %f\n", magnitude);
        if (magnitude * .99 > S_E_LENGTH + E_W_LENGTH) {//check if in range
            return false;
        }

        theta_1 = law_of_cosines(E_W_LENGTH, magnitude, S_E_LENGTH);
        theta_2 = law_of_cosines(S_E_LENGTH, magnitude, E_W_LENGTH);
        // printf("Thetas: %d, %d", to_deg(theta_1), to_deg(theta_2));

        shoulder_rad = theta_1 + atan(y_pos / x_pos);
        elbow_rad = PI - theta_1 - theta_2;

        *shoulder_angle = to_deg(PI - shoulder_rad);
        *elbow_angle = to_deg(elbow_rad);
        *wrist_angle = to_deg(WRIST_CONST + shoulder_rad + elbow_rad);
        return true;
    }