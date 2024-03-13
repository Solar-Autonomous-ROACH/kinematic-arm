# Examples

# Example 1
This example showcases the arm functionality combined with vision functionality. It repeatedly does sample tube pickups until the program is exited.
If you want to use the vision dummy (you can actually enter your own coordinates) instead of the real dummy program, make the library in this way (you need to make clean if you already compiled library without VISION_DUMMY flag):
```sh
VISION_DUMMY=1 make
```

# Example 2
Same as example 1 but also integrates with rover move commands.

# Example Hall Effect (1 & 2)
Might be outdated. Test hall effect functionality. Understand the code before your run. You need to make sure that hall effect is connect and is actually loaded to the FPGA.