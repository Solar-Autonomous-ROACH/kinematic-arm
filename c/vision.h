#ifndef VISION_H
#define VISION_H

#include <signal.h>

typedef struct
{
    int16_t x;  //centimeters
    int16_t y;
    int16_t z;
    int16_t angle;  //0 to 179
}  vision_info_t;


void vision_init();
void vision_request_coordinates();
vision_status_t vision_receive_coordinates();
vision_status_t vision_get_status();
vision_info_t *vision_get_coordinates();

#endif