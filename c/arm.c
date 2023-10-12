// #include "arm.h"

// void arm_init(){
//     steer_FR.index = BASE;
//     steer_FR.state = STATE_INITIALIZE;

//     steer_RR.index = ELBOW;
//     steer_RR.state = STATE_INITIALIZE;

//     steer_FL.index = WRIST;
//     steer_FL.state = STATE_INITIALIZE;

//     steer_RL.index = CLAW;
//     steer_RL.state = STATE_INITIALIZE;

//     steering_motor_handle_state(&steer_BASE);
//     steering_motor_handle_state(&steer_ELBOW);
//     steering_motor_handle_state(&steer_WRIST);
//     steering_motor_handle_state(&steer_CLAW);

//     arm_state = ARM_CALIBRATE_WAITING;
// }

// int arm_is_calibrated() {
//     return arm_state == ARM_CALIBRATE_READY;
// }

// void arm_calibrate(){
//     switch (arm_state) {
//         case ARM_CALIBRATE_WAITING:
//             calibrate(&steer_BASE);
//             arm_state = ARM_CALIBRATE_BASE;
//             break;
//         case ARM_CALIBRATE_BASE:
//             if (steering_motor_handle_state(&steer_BASE)) {
//                 calibrate(&steer_RR);
//                 arm_state = ARM_CALIBRATE_ELBOW;
//             }
//             break;
//         case ARM_CALIBRATE_ELBOW:
//             if (steering_motor_handle_state(&steer_ELBOW)) {
//                 calibrate(&steer_FL);
//                 arm_state = ARM_CALIBRATE_WRIST;
//             }
//             break;
//         case ARM_CALIBRATE_WRIST:
//             if (steering_motor_handle_state(&steer_WRIST)) {
//                 calibrate(&steer_RL);
//                 arm_state = ARM_CALIBRATE_CLAW;
//             }
//             break;
//         case ARM_CALIBRATE_CLAW:
//             if (steering_motor_handle_state(&steer_CLAW)) {
//                 // rover_steer_forward();
//                 arm_state = ARM_CALIBRATE_READY;
//             }
//             break;
//         case ARM_CALIBRATE_READY:
//             break;
//         default:
//             break;
//     }
// }

// // void rover_update_steering(){
// //     steering_motor_handle_state(&steer_FR);
// //     steering_motor_handle_state(&steer_RR);
// //     steering_motor_handle_state(&steer_FL);
// //     steering_motor_handle_state(&steer_RL);
// // }

// // void rover_stop(){
// //     set_motor_speed(FRW, 0);
// //     set_motor_speed(RRW, 0);
// //     set_motor_speed(FLW, 0);
// //     set_motor_speed(RLW, 0);
// //     set_motor_speed(MRW, 0);
// //     set_motor_speed(MLW, 0);
// // }

// // void rover_forward(int speed){
// //     set_motor_speed(FRW, -speed);
// //     set_motor_speed(MRW, -speed);
// //     set_motor_speed(RRW, -speed);
// //     set_motor_speed(FLW, speed);
// //     set_motor_speed(MLW, speed);
// //     set_motor_speed(RLW, speed);
// // }

// // void rover_reverse(int speed){
// //     set_motor_speed(FRW, speed);
// //     set_motor_speed(MRW, speed);
// //     set_motor_speed(RRW, speed);
// //     set_motor_speed(FLW, -speed);
// //     set_motor_speed(MLW, -speed);
// //     set_motor_speed(RLW, -speed);
// // }

// // void rover_pointTurn_CW(int speed){
// //     set_motor_speed(FRW, speed);
// //     set_motor_speed(MRW, speed);
// //     set_motor_speed(RRW, speed);
// //     set_motor_speed(FLW, speed);
// //     set_motor_speed(MLW, speed);
// //     set_motor_speed(RLW, speed);
// // }

// // void rover_pointTurn_CCW(int speed){
// //     set_motor_speed(FRW, -speed);
// //     set_motor_speed(MRW, -speed);
// //     set_motor_speed(RRW, -speed);
// //     set_motor_speed(FLW, -speed);
// //     set_motor_speed(MLW, -speed);
// //     set_motor_speed(RLW, -speed);
// // }

// // void rover_steer_forward(){
// //     steer_FR.target = steer_FR.center_pos + 0;
// //     steer_FL.target = steer_FL.center_pos + 0;
// //     steer_RR.target = steer_RR.center_pos + 0;
// //     steer_RL.target = steer_RL.center_pos + 0;
// // }

// // void rover_steer_right(int angle){
// //     if(angle >  MAX_STEERING_TICKS) angle =    MAX_STEERING_TICKS;
// //     if(angle < -MAX_STEERING_TICKS) angle =   -MAX_STEERING_TICKS;
// //     steer_FR.target = steer_FR.center_pos + angle;
// //     steer_RR.target = steer_RR.center_pos - angle;
// //     steer_FL.target = steer_FL.center_pos + angle;
// //     steer_RL.target = steer_RL.center_pos - angle;
// // }

// // void rover_steer_left(int angle){
// //     if(angle >  MAX_STEERING_TICKS) angle =  MAX_STEERING_TICKS;
// //     if(angle < -MAX_STEERING_TICKS) angle = -MAX_STEERING_TICKS;
// //     steer_FR.target = steer_FR.center_pos -  angle;
// //     steer_RR.target = steer_RR.center_pos +  angle;
// //     steer_FL.target = steer_FL.center_pos -  angle;
// //     steer_RL.target = steer_RL.center_pos +  angle;
// // }

// // void rover_steer_point(){
// //     steer_FR.target = steer_FR.center_pos - MAX_STEERING_TICKS;
// //     steer_RR.target = steer_RR.center_pos + MAX_STEERING_TICKS;
// //     steer_FL.target = steer_FL.center_pos + MAX_STEERING_TICKS;
// //     steer_RL.target = steer_RL.center_pos - MAX_STEERING_TICKS;
// // }

