
#include "kinematic_engine.h"

double law_of_cosines(double a, double b, double c) {
    return acos((b * b + c * c - a * a) / (2 * b * c));
}


int to_deg(double radians) {
    return (int) (radians * RAD_TO_DEG);
}


bool kinematic_engine(float x_pos, float y_pos, float z_pos, 
    int * shoulder_angle, int * elbow_angle, int * wrist_angle, int * turn_angle) {
        // printf("Values: %f, %f, %f\n", x_pos, y_pos, z_pos);
        double magnitude, theta_1, theta_2, shoulder_rad, elbow_rad;
        clock_t start_time, end_time;

        start_time = clock();

        if (!((0 - SMALL_DOUBLE) <= z_pos && z_pos <= SMALL_DOUBLE)) {
            //If Z is something other than 0
            *turn_angle = to_deg(atan(z_pos / x_pos));
            x_pos = sqrt(x_pos * x_pos + z_pos * z_pos);
        } else {
            //Set turn angle to 0 otherwise, for consistency
            *turn_angle = 0;
        }

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

        end_time = clock();
        printf("Engine Elapsed: %.1f ms\n", (((double)(end_time - start_time)) * 1000.0 / CLOCKS_PER_SEC);
        return true;
    }