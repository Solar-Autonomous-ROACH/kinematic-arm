
#include "kinematic_engine.h"

double law_of_cosines(double a, double b, double c) {
    return acos((b * b + c * c - a * a) / (2 * b * c));
}


int to_deg(double radians) {
    return (int) (radians * RAD_TO_DEG)
}


bool kinematic_engine(float x_pos, float y_pos, float z_pos, 
    int * shoulder_angle, int * elbow_angle, int * wrist_angle) {
        double magnitude;
        double theta_1;
        double theta_2;

        y_pos -= E_W_LENGTH;

        magnitude = sqrt(x_pos * x_pos + y_pos * y_pos);
        if (magnitude * .99 > S_E_LENGTH + E_W_LENGTH) {//check if in range
            return false
        }

        theta_1 = law_of_cosines(S_E_LENGTH, magnitude, E_W_LENGTH);
        theta_2 = law_of_cosines(E_W_LENGTH, magnitude, S_E_LENGTH);

        *shoulder_angle = 180 - to_deg(theta_1 + atan(y_pos / x_pos));
        *elbow_angle = to_deg(PI -  theta_1 - theta_2);
        *wrist_angle = 280 - to_deg(3 * PI / 2 - *shoulder_angle - *elbow_angle)
        return true;
    }