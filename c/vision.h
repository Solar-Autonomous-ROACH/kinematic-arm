#ifndef VISION_H
#define VISION_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  int16_t x; // centimeters
  int16_t y;
  int16_t z;
  int16_t angle; // 0 to 179
} vision_info_t;



typedef enum {
  VISION_TERMINATED,
  VISION_STARTING,
  VISION_READY_FOR_CAPTURE,
  VISION_IN_PROGRESS,
  VISION_SAMPLE_NOT_FOUND,
  VISION_SUCCESS,
  VISION_ERROR
} vision_status_t;

void vision_init();
void vision_request_coordinates();
vision_status_t vision_receive_input();
vision_status_t vision_receive_input_isr();
vision_status_t vision_get_status();
bool vision_get_coordinates(vision_info_t *v);
void vision_terminate(bool wait);

#endif