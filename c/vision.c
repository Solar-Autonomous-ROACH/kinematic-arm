#include "vision.h"
#include <bool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>

pid_t vision_pid;
FILE *vision_stdout;
vision_info_t *vision_info;
vision_status_t vision_state;

#define READ_END 0
#define WRITE_END 1

int pipefd[2];

void vision_init() {
  if (pipe(pipefd) == -1) {
    perror("pipe vision");
    exit(1);
  }

  vision_pid = fork();

  if (vision_pid == -1) {
    perror("fork vision");
    exit(1);
  } else if (vision_pid == 0) { /** child */
    dup2(pipefd[WRITE_END], STDOUT_FILENO);
    close(pipefd[READ_END]);

    if (execlp("python3", "../python/vision.py", NULL) == -1) {
      perror("exec vision") exit(1);
    }

  } else {
    FILE *vision_stdout = fdopen(pipefd[READ_END], 'r');
    close(pipefd[WRITE_END]);
  }
}

void vision_request_coordinates() {
  vision_state = VISION_IN_PROGRESS;
  kill(vision_pid, SIGUSR1);
}

vision_status_t vision_receive_coordinates() {
  char c = getchar();
  if (c == EOF) {
    return vision_state;
  }
  if (c == 'x') {
    // x=1,y=20,z=2,a=5
    if (scanf("=%d,y=%d,z=%d,a=%d\n", &(vision_info.x), &(vision_info.y),
              &(vision_info.z), &(vision_info.angle)) > 0) {
      vision_state = VISION_SUCCESS
    }
  } else if (c == '0') {
    vision_state = VISION_SAMPLE_NOT_FOUND;
  } else {
    vision_state = VISION_SAMPLE_NOT_FOUND;
  }
  return vision_state;
}

vision_status_t vision_receive_coordinates_isr() {
  struct pollfd fds[1];
  int read_char;
  fds[0].fd = STDIN_FILENO;
  fds[0].events = POLLIN;

  read_char = poll(fds, 1, 0);
  if (read_char == -1) {
    perror("poll error");
    exit(1);
  } else if (read_char > 0) {
    char c;
    scanf("%c", &c);
    if (c == 'x') {
      // x=1,y=20,z=2,a=5
      if (scanf("=%d,y=%d,z=%d,a=%d\n", &(vision_info.x), &(vision_info.y),
                &(vision_info.z), &(vision_info.angle)) > 0) {
        vision_state = VISION_SUCCESS
      }
    } else if (c == '0') { //no test tube found by vision
      vision_state = VISION_SAMPLE_NOT_FOUND;
    } else {  //vision outputted something unexpected
      vision_state = VISION_ERROR;
    }
  } else { //no input to read (vision engine still running)
    vision_state = VISION_SAMPLE_NOT_FOUND;
  }
  return vision_state;
}

vision_status_t vision_get_status() { return vision_state; }

vision_info_t *vision_get_coordinates() { return vision_info; }
