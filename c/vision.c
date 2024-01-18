#include "vision.h"
#include "logger.h"
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

pid_t vision_pid;
int vision_fd;
FILE *vision_stdout;
vision_info_t vision_info;
vision_status_t vision_state = VISION_READY_FOR_CAPTURE;

struct pollfd fds[1];

struct sigaction sa;

#define READ_END 0
#define WRITE_END 1

int pipefd[2];

void handler(int signo) {
  printf("RIP\n");
  // do some cleanup
  // wait
  // close file descriptors
  // change status to VISION_TERMINATED
  //
}

void vision_init() {
  if (pipe(pipefd) == -1) {
    perror("pipe vision");
    exit(1);
  }

  sa.sa_handler = handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  sigaddset(&sa.sa_mask, SIGALRM);
  sigaction(SIGALRM, &sa, NULL);

  vision_pid = fork();

  if (vision_pid == -1) {
    perror("fork vision");
    exit(1);
  } else if (vision_pid != 0) { /** parent */
    vision_fd = pipefd[READ_END];
    fds[0].fd = vision_fd;
    fds[0].events = POLLIN;
    FILE *vision_stdout = fdopen(pipefd[READ_END], 'r');
    close(pipefd[WRITE_END]);
    vision_state = VISION_READY_FOR_CAPTURE;
  } else {
    dup2(pipefd[WRITE_END], STDOUT_FILENO);
    close(pipefd[READ_END]);

    if (execlp("python3", "../python/vision.py", NULL) == -1) {
      perror("exec vision");
      exit(1);
    }
  }
}

void vision_request_coordinates() {
  vision_state = VISION_IN_PROGRESS;
  if (kill(vision_pid, SIGUSR1) == -1) {
    perror("kill vision");
    exit(1);
  }
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
  int poll_out;

  poll_out = poll(fds, 1, 0);
  if (poll_out == -1) {
    perror("poll error");
    exit(1);
  } else if (poll_out > 0) {
    char c;
    scanf("%c", &c);
    if (c == 'x') {
      // x=1,y=20,z=2,a=5
      if (scanf("=%d,y=%d,z=%d,a=%d\n", &(vision_info.x), &(vision_info.y),
                &(vision_info.z), &(vision_info.angle)) > 0) {
        vision_state = VISION_SUCCESS;
      }
    } else if (c == '0') { // no test tube found by vision
      vision_state = VISION_SAMPLE_NOT_FOUND;
    } else { // vision outputted something unexpected
      log_message(LOG_DEBUG, "Vision error\n");
      vision_state = VISION_ERROR;
    }
  }

  return vision_state;
}

vision_status_t vision_get_status() { return vision_state; }

vision_info_t *vision_get_coordinates() { return &vision_info; }
