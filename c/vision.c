#include "vision.h"
#include "logger.h"
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

pid_t vision_pid;
int vision_fd;
FILE *vision_stdout;
vision_info_t vision_info;
vision_status_t vision_state = VISION_TERMINATED;

struct pollfd fds[1];

struct sigaction sa;

#define READ_END 0
#define WRITE_END 1

int pipefd[2];

void handler(int signo) {
  int status;
  pid_t wait_ret = waitpid(vision_pid, &status, WNOHANG);
  if (wait_ret == -1) {
    perror("waitpid vision");
    exit(1);
  } else if (wait_ret > 0) {
    fclose(vision_stdout);
    close(vision_fd);
    vision_state = VISION_TERMINATED;
  }
}

void vision_init() {
  // only init if terminated
  if (vision_state != VISION_TERMINATED) {
    return;
  }
  /** pipe for getting input from vision */
  if (pipe(pipefd) == -1) {
    perror("pipe vision");
    exit(1);
  }

  vision_pid = fork();

  if (vision_pid == -1) {
    perror("fork vision");
    exit(1);
  } else if (vision_pid != 0) { /** parent */
    // handle vision termination
    sa.sa_handler = handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    // sigaddset(&sa.sa_mask, SIGALRM);
    sigaction(SIGCHLD, &sa, NULL);
    // plumbing work
    vision_fd = pipefd[READ_END];
    close(pipefd[WRITE_END]);
    // setup poll
    fds[0].fd = vision_fd;
    fds[0].events = POLLIN;
    // use stdio FILE instead of fd
    vision_stdout = fdopen(pipefd[READ_END], 'r');
    vision_state = VISION_STARTING;
  } else {
    dup2(pipefd[WRITE_END], STDOUT_FILENO);
    close(pipefd[READ_END]);

    if (execlp("python3", "../python/vision.py", NULL) == -1) {
      perror("exec vision");
      exit(1);
    }
  }
}

/**
 * @brief Send a request to vision for coordinates
 * only sends it if vision state is VISION_READY_FOR_CAPTURE
 */
void vision_request_coordinates() {
  if (vision_state == VISION_READY_FOR_CAPTURE ||
      vision_state == VISION_SAMPLE_NOT_FOUND ||
      vision_state == VISION_SUCCESS) {
    if (kill(vision_pid, SIGUSR1) == -1) {
      perror("kill vision");
      exit(1);
    }
    vision_state = VISION_IN_PROGRESS;
  }
}

/**
 * @brief Designed to run outside of ISR - blocking
 * checks for data coming in from vision
 *
 * @return vision_status_t
 */
vision_status_t vision_receive_input() {
  char c = fgetc(vision_stdout);
  if (c == EOF) {
    return vision_state;
  }
  switch (c) {
  case 'r': /** vision startup complete */
    vision_state = VISION_READY_FOR_CAPTURE;
    break;

  case 'x': /** coordinates start with x */
    if (fscanf(vision_stdout, "=%d,y=%d,z=%d,a=%d\n", &(vision_info.x),
               &(vision_info.y), &(vision_info.z), &(vision_info.angle)) > 0) {
      vision_state = VISION_SUCCESS;
    } else {
      vision_state = VISION_ERROR;
    }
    break;

  case '0': /** no sample tube found */
    vision_state = VISION_SAMPLE_NOT_FOUND;
    break;

  default:
    log_message(LOG_DEBUG, "Vision error\n");
    vision_state = VISION_ERROR;
    break;
  }
  return vision_state;
}

/**
 * @brief Designed to be run in the ISR - nonblocking
 * checks for data coming in from vision
 *
 * @return vision_status_t
 */
vision_status_t vision_receive_input_isr() {
  int poll_out;

  poll_out = poll(fds, 1, 0);
  if (poll_out == -1) {
    perror("poll error");
    exit(1);
  } else if (poll_out > 0) {
    vision_state = vision_receive_input();
  }

  return vision_state;
}

vision_status_t vision_get_status() { return vision_state; }

/**
 * @brief returns the coordinates of sample tube from vision
 *
 * @return vision_info_t* coordinates if found. NULL if not found
 */
vision_info_t *vision_get_coordinates() {
  if (vision_state == VISION_SUCCESS) {
    vision_state = VISION_READY_FOR_CAPTURE;
    return &vision_info;
  }
  if (vision_state != VISION_SAMPLE_NOT_FOUND) {
    vision_state = VISION_READY_FOR_CAPTURE;
  }
  return NULL;
}

/**
 * @brief terminates vision process
 *
 */
void vision_terminate(bool wait) {
  kill(vision_pid, SIGTERM);
  if (wait) {
    int status;
    pid_t wait_ret = waitpid(vision_pid, &status, 0);
    if (wait_ret == -1) {
      perror("waitpid vision");
      exit(1);
    } else if (wait_ret > 0) {
      fclose(vision_stdout);
      close(vision_fd);
      vision_state = VISION_TERMINATED;
    }
  }
}
