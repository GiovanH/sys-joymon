#include <switch.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include "log.h"
#include <time.h>

using namespace std;

//Matrix containing the name of each key. Useful for printing when a key is pressed
char keysNames[32][32] = {
	"KEY_A", "KEY_B", "KEY_X", "KEY_Y",
	"KEY_LSTICK", "KEY_RSTICK", "KEY_L", "KEY_R",
	"KEY_ZL", "KEY_ZR", "KEY_PLUS", "KEY_MINUS",
	"KEY_DLEFT", "KEY_DUP", "KEY_DRIGHT", "KEY_DDOWN",
	"KEY_LSTICK_LEFT", "KEY_LSTICK_UP", "KEY_LSTICK_RIGHT",
	    "KEY_LSTICK_DOWN",
	"KEY_RSTICK_LEFT", "KEY_RSTICK_UP", "KEY_RSTICK_RIGHT",
	    "KEY_RSTICK_DOWN",
	"KEY_SL", "KEY_SR", "KEY_TOUCH", "",
	"", "", "", ""
};

ofstream joyLogFile;
u64 frame = 0;
clock_t startt;

u32 kDownOld = 0, kHeldOld = 0, kUpOld = 0;	// Previous state
u32 kDown = 0, kHeld = 0, kUp = 0;
int logFileIndex = 0;
bool logging = false;

void initLogs()
{
	mkdir("sdmc:/logs", 0700);
	joyLogFile = ofstream("sdmc:/logs/joy.log");
}

void closeLogs(){
	joyLogFile.close();
}

void writeHidEntry()
{
	hidScanInput();

	kDown = hidKeysDown(CONTROLLER_P1_AUTO);
	kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);
	kUp = hidKeysUp(CONTROLLER_P1_AUTO);

	if (kDown != kDownOld || kHeld != kHeldOld || kUp != kUpOld) {
		int i;
		for (i = 0; i < 32; i++) {
			if (kDown & BIT(i)) (joyLogFile << "D" << keysNames[i] << i << endl);
			if (kHeld & BIT(i)) (joyLogFile << "H" << keysNames[i] << i << endl);
			if (kUp & BIT(i))   (joyLogFile << "U" << keysNames[i] << i << endl);
		}
	}
	//Remember one frame back
	kDownOld = kDown;
	kHeldOld = kHeld;
	kUpOld = kUp;

	JoystickPosition pos_left, pos_right;

	hidJoystickRead(&pos_left, CONTROLLER_P1_AUTO, JOYSTICK_LEFT);
	hidJoystickRead(&pos_right, CONTROLLER_P1_AUTO, JOYSTICK_RIGHT);

	joyLogFile << "JLX" << pos_left.dx << "Y" << pos_left.dy << endl;
	joyLogFile << "JRX" << pos_right.dx << "Y" << pos_right.dy << endl;
	joyLogFile << endl;
}

void setLogging(bool newVal)
{
	logging = newVal;
	frame = 0;
	// Either we just started, or we're wrapping up. Either way,
	if (logging) {
		joyLogFile = ofstream("sdmc:/logs/joy" + to_string(++logFileIndex) + ".log");
		startt = clock();
	} else {
		joyLogFile.close();
	}
}

void inputPoller()
{
	hidScanInput();
	u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

	// Toggle recording with <- + ->
	if ((kDown & KEY_DLEFT) && (kDown & KEY_DRIGHT)) {
		setLogging(!logging);
	}

	if (logging) {
		joyLogFile << "SEC" << clock() << "-" << startt << "=" << (clock()-startt) << endl;
		joyLogFile << "F" << frame++ << endl;
		writeHidEntry();
		if (frame > 1000) {
			// Stop if too long
			setLogging(false);
		}
	}
	//svcSleepThread(3000000L);
}
