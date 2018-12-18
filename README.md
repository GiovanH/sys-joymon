# sys-joymon

SysModule.hpp headers from XorTroll.

json template from libnx-examples.



This is designed to be used with `sys-repeater`.



## Documentation

Once loaded as a sysmodule, you can click the two stick buttons down simultaneously to begin recording. The switch will emit a friendly beep. Pressing the two sticks down again will stop the recording.

Recording files are output in `sdmc:/joy/c/joy#.c`, where `joy#.c` follows the steps.c format, explained below.

When recording, the module will wait `framelength` nanoseconds between frames. This framerate can be adjusted by putting an integer in the plaintext file `sdmc:/joy/config_framelength`. This can be adjusted on the fly. 



### steps.c format (Copied from `SysRepeater`)

Programs must have data compiled in; there is no simple way to read data from file. Thus, the series of steps must be compiled into the executable. The data format used is specifically designed to compress efficiently enough to fit onto the Teensy 2.0.

Each step can be thought of as a `joystickreport` struct, as such:

```
typedef struct {
	uint16_t Button; // 16 buttons; see JoystickButtons_t for bit mapping
	uint8_t  HAT;    // HAT switch; one nibble w/ unused nibble
	uint8_t  LX;     // Left  Stick X
	uint8_t  LY;     // Left  Stick Y
	uint8_t  RX;     // Right Stick X
	uint8_t  RY;     // Right Stick Y
	uint8_t  VendorSpec;
} USB_JoystickReport_t;
```

*However,* in order to optimize cross-compatibility, this is saved in the form of a 5-tuple of `uint16`s. The following is a valid steps.c file:

```uint16_t step[] = {
uint16_t step[] = {
    1024, 32896, 32896, 8, 26,  //Step 1
    0, 37885, 32896, 8, 0,      //Step 2
    0, 34777, 32896, 8, 0,      //Step 3
    0, 32896, 32896, 8, 11, 
    4, 32896, 32896, 8, 3,
    0, 32896, 32896, 8, 28,
0 } // Extra data ignored
static int numsteps = 6 // One-indexed.
```

The data stored, respectively, is

1. `Button`: A 16-bit uint that acts as a binary flag, selecting a combination of 16 buttons.
2. `lData`: Data for the left joystick. The 16-bit unsigned integer is actually storage for two 8-bit unsigned integers, prepended together, representing the X and Y axis. 
3. `rData`: See 2. 
4. `hData`: Two 8-bit uints, as with 2 and 3. The lower portion represents the HAT, while the upper portion represents the VendorSpec data.
5. `Repeats`: Each step, even if repeats is 0, is sent at least once. If repeats is greater than 0, the step is repeated before moving on. A step with 31 repeats, for instance, will be sent 32 times. (This does not include the echoes, used to timescale the data.)

See log.cpp for full implementation. 