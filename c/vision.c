#include "vision.h"
#include "logger.h"
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

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

void handler(int signo __attribute__((unused))) {
  int status;
  pid_t wait_ret = waitpid(vision_pid, &status, WNOHANG);
  if (wait_ret == -1) {
    perror("waitpid vision");
    raise(SIGINT);
  } else if (wait_ret > 0) {
    fclose(vision_stdout);
    close(vision_fd);
    vision_state = VISION_TERMINATED;
  }
}

void vision_init() {
  // only init if terminated or errored out
  if (vision_state != VISION_TERMINATED && vision_state != VISION_ERROR) {
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
    log_message(LOG_INFO, "Opened vision fd %d\n", vision_fd);
    // setup poll
    fds[0].fd = vision_fd;
    fds[0].events = POLLIN;
    // use stdio FILE instead of fd
    vision_stdout = fdopen(pipefd[READ_END], "r");
    if (vision_stdout == NULL) {
      perror("fdopen vision");
      exit(1);
    }
    vision_state = VISION_STARTING;
    log_message(LOG_INFO, "Vision init complete\n");
  } else {
    dup2(pipefd[WRITE_END], STDOUT_FILENO);
    close(pipefd[READ_END]);

    if (execlp("/home/xilinx/arm_2024/python/vision.py", "vision.py", NULL) ==
        -1) {
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
      raise(SIGINT);
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
  int c = fgetc(vision_stdout);
  int dummy;
  if (c == EOF) {
    return vision_state;
  }
  log_message(LOG_INFO, "Received from vision %c\n", c);
  switch (c) {
  case 'r': /** vision startup complete */
    vision_state = VISION_READY_FOR_CAPTURE;
    dummy = fgetc(vision_stdout); // get rid of extra newline
    if (dummy != '\n') {
      log_message(LOG_ERROR, "Vision error\n");
      vision_state = VISION_ERROR;
    }
    break;

  case 'x': /** coordinates start with x */
    // fscanf(vision_stdout, "=%hd,", &(vision_info.x));
    // log_message(LOG_INFO, "x = %hd\n", vision_info.x);
    // fscanf(vision_stdout, "y=%hd,", &(vision_info.y));
    // log_message(LOG_INFO, "y = %hd\n", vision_info.y);
    // fscanf(vision_stdout, "z=%hd,", &(vision_info.z));
    // log_message(LOG_INFO, "z = %hd\n", vision_info.z);
    // fscanf(vision_stdout, "a=%hd\n", &(vision_info.angle));
    // log_message(LOG_INFO, "a = %hd\n", vision_info.angle);
    // exit(0);
    if (fscanf(vision_stdout, "=%hd,y=%hd,z=%hd,a=%hd,c=%lf", &(vision_info.x),
               &(vision_info.y), &(vision_info.z), &(vision_info.angle),
               &(vision_info.confidence)) > 0) {
      dummy = fgetc(vision_stdout); // get rid of extra newline
      if (dummy != '\n') {
        log_message_line(LOG_ERROR, __LINE__, "Vision error\n");
      }
      log_message(LOG_INFO, "x=%hd,y=%hd,z=%hd,a=%hd,c=%lf\n", vision_info.x,
                  vision_info.y, vision_info.z, vision_info.angle,
                  vision_info.confidence);
      vision_state = VISION_SUCCESS;
    } else {
      log_message(LOG_INFO, "vision fscan error\n");
      vision_state = VISION_ERROR;
    }
    break;

  case '0': /** no sample tube found */
    vision_state = VISION_SAMPLE_NOT_FOUND;
    dummy = fgetc(vision_stdout); // get rid of edxtra
    if (dummy != '\n') {
      log_message_line(LOG_ERROR, __LINE__, "Vision error\n");
    }
    break;

  default:
    log_message_line(LOG_ERROR, __LINE__, "Vision error\n");
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
    raise(SIGINT);
  } else if (poll_out > 0) {
    // log_message(LOG_INFO, "read character from poll\n");
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
bool vision_get_coordinates(vision_info_t *v) {
  if (vision_state == VISION_SUCCESS) {
    vision_state = VISION_READY_FOR_CAPTURE;
    v->x = vision_info.x;
    v->y = vision_info.y;
    v->z = vision_info.z;
    v->angle = vision_info.angle;
    v->confidence = vision_info.confidence;
    return true;
  }
  if (vision_state != VISION_SAMPLE_NOT_FOUND) {
    vision_state = VISION_READY_FOR_CAPTURE;
  }
  return false;
}

/**
 * @brief terminates vision process
 *
 */
void vision_terminate(bool wait) {
  if (wait) {
    // first disable SIGCHLD handler
    sa.sa_handler = SIG_DFL; // Set to default handler
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGCHLD, &sa, NULL);
    kill(vision_pid, SIGTERM);
    int status;
    pid_t wait_ret = waitpid(vision_pid, &status, 0);
    if (wait_ret == -1) {
      perror("waitpid vision");
      raise(SIGINT);
    } else if (wait_ret > 0) {
      fclose(vision_stdout);
      close(vision_fd);
      vision_state = VISION_TERMINATED;
    }
  } else {
    // just kill SIGCHLD handler will handler the rest
    kill(vision_pid, SIGTERM);
  }
}
