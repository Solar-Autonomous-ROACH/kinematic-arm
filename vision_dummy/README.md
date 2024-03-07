# Vision team, take a look at basic_example.py

## vision.py has extra features for Armada team
When vision.py receive a SIGUSR1, it waits for user input from stdin. There are 4 accepted inputs:
1. a new set of coordinates to return: e.g., x=100,y=-200,z=300,a=40
2. 'a' to add 100 to y coordinate. This is useful to simulate vision system when validating pickup.
3. 's' to subtract 100 to y coordinate. This is useful to return back to orignal coordinates.
4. 'n' for no change. Usee the already set coordinates
