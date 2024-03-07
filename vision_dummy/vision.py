#!/usr/bin/python3
from time import sleep
import signal
import os
import re
import sys

STARTUP_TIME = 1
CAPTURE_TIME = 5

coords = {"x": 300, "y": -340, "z": 0, "a": 0, "c": 0.56}  # default coords


class bcolors:
    HEADER = "\033[95m"
    OKBLUE = "\033[94m"
    OKCYAN = "\033[96m"
    OKGREEN = "\033[92m"
    WARNING = "\033[93m"
    FAIL = "\033[91m"
    ENDC = "\033[0m"
    BOLD = "\033[1m"
    UNDERLINE = "\033[4m"


def handler(signum, frame):
    signame = signal.Signals(signum).name
    print(
        f"{bcolors.FAIL}Signal handler called with signal {signame} ({signum}){bcolors.ENDC}",
        file=sys.stderr,
    )


print(f"{bcolors.FAIL}Vision PID: {os.getpid()}{bcolors.ENDC}", file=sys.stderr)

sleep(STARTUP_TIME)  # simulating startup
print("r", flush=True)

# Set the signal handler and a 5-second alarm
signal.signal(signal.SIGUSR1, handler)


while True:
    signal.pause()
    print(
        f"""{bcolors.FAIL}(Current: x={coords['x']},y={coords['y']},z={coords['z']},a={coords['a']},c={coords['c']})
    Enter:
        1. a new set of coordinates: e.g., x=1,y=2,z=3,a=4,c=0.0
        2. 'a' to add 100 to y coordinate
        2. 's' to subtract 100 to y coordinate
        4. 'n' for no change
        5. '0' no test tube found
        {bcolors.ENDC}""",
        file=sys.stderr,
    )
    input_str = input()
    if input_str[0] == "x":
        # Extracting values using string manipulation
        split_str = input_str.split(",")
        x_str = split_str[0].split("=")[1]
        y_str = split_str[1].split("=")[1]
        z_str = split_str[2].split("=")[1]
        a_str = split_str[3].split("=")[1]
        c_str = split_str[4].split("=")[1]

        # Converting strings to short integers
        coords["x"] = int(x_str)
        coords["y"] = int(y_str)
        coords["z"] = int(z_str)
        coords["a"] = int(a_str)
        coords["c"] = float(c_str)

    elif input_str[0].lower() == "a":  # for verify
        coords["y"] += 100
    elif input_str[0].lower() == "s":  # for pickup
        coords["y"] -= 100
    elif input_str[0].lower() == "0":  # for no sample tube found
        print("0", flush=True)
        continue
    elif input_str[0].lower() == "n":  # no change
        pass

    print(
        f"x={coords['x']},y={coords['y']},z={coords['z']},a={coords['a']},c={coords['c']}",
        flush=True,
    )
