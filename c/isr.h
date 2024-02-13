//
// Created by Tyler Bovenzi on 3/23/23.
//

#include <signal.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include "mmio.h"
#include "stdint.h"

#ifndef ROVERCORE_ISR_H
#define ROVERCORE_ISR_H

int isr_init();

int isr(int signum);

#endif // ROVERCORE_ISR_H
