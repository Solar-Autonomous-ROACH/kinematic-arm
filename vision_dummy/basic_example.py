#!/usr/bin/python3
from time import sleep
import signal
import os
import sys

STARTUP_TIME = 10
CAPTURE_TIME = 5

coords = {"x": 300, "y": -340, "z": 0, "a": 0}  # default coords


def handler(signum, frame):
    signame = signal.Signals(signum).name
    print(f"Signal handler called with signal {signame} ({signum})", file=sys.stderr)


print(os.getpid(), file=sys.stderr)

sleep(STARTUP_TIME)  # simulating startup
print("r", flush=True)


# Set the signal handler and a 5-second alarm
signal.signal(signal.SIGUSR1, handler)


while True:
    signal.pause()
    sleep(CAPTURE_TIME)  # simulate time for capture
    print(
        f"x={coords['x']},y={coords['y']},z={coords['z']},a={coords['a']}", flush=True
    )
