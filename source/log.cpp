#include <switch.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include "log.h"
#include <time.h>
#include "mp3.h"

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

uint16_t keyCodes[32] = {
	0x04,  0x02,  0x08,  0x01, 
	0x400, 0x800, 0x10,  0x20,
	0x40,  0x80,  0x200, 0x100, 
	//Hat
	0x06, 0x00, 0x02, 0x04, 
	0x0,  0x0,  0x0,  0x0, 
	0x0,  0x0,  0x0,  0x0, 
	0x0,  0x0,  0x0,  0x0, 
	0x0,  0x0,  0x0,  0x0, 
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
}

void closeLogs(){
	joyLogFile.close();
}

uint16_t uButtons = 0, uLeft = 0, uRight = 0, uHat = 0x08;

void writeHidEntry()
{
	hidScanInput();

	kDown = hidKeysDown(CONTROLLER_P1_AUTO);
	kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);
	kUp = hidKeysUp(CONTROLLER_P1_AUTO);

	uButtons = 0;
	uLeft = 0;
	uRight = 0;
	uHat = 0x08;

	//if (kDown != kDownOld || kHeld != kHeldOld || kUp != kUpOld) {
		int i;
		//Buttons
		for (i = 0; i < 12; i++) {
			if (kHeld & BIT(i)) uButtons |= keyCodes[i];
			//if (kDown & BIT(i)) uButtons |= keyCodes[i];
			//if (kUp & BIT(i)) uButtons ^= keyCodes[i];
			if (kDown & BIT(i)) (joyLogFile << "//Down " << keysNames[i] << i << endl);
			if (kHeld & BIT(i)) (joyLogFile << "//Held " << keysNames[i] << i << endl);
			if (kUp & BIT(i))   (joyLogFile << "//Up " << keysNames[i] << i << endl);
		}
		//Hat
		for (;i < 16; i++) { 
			if (kHeld & BIT(i)) uHat = keyCodes[i];
			if (kDown & BIT(i)) (joyLogFile << "//Down " << keysNames[i] << i << endl);
		}
		/*
		//LStick
		for (i < 20; i++) { 
			
		}
		//RStick
		for (i < 24; i++) { 
			
		}
		*/
	//}
	//Remember one frame back
	kDownOld = kDown;
	kHeldOld = kHeld;
	kUpOld = kUp;


    JoystickPosition pos_left, pos_right;

    //Read the joysticks' position
    hidJoystickRead(&pos_left, CONTROLLER_P1_AUTO, JOYSTICK_LEFT);
    hidJoystickRead(&pos_right, CONTROLLER_P1_AUTO, JOYSTICK_RIGHT);

    uLeft += (uint8_t)(pos_left.dx/128);
    uLeft += (uint8_t)(pos_left.dy/64);
    uRight += (uint8_t)(pos_right.dx/128);
    uRight += (uint8_t)(pos_right.dy/64);

	joyLogFile << "    " << uButtons << ", " << uLeft << ", " << uRight << ", " << endl;
}

void setLogging(bool newVal)
{
	logging = newVal;
	frame = 0;
	// Either we just started, or we're wrapping up. Either way,
	if (logging) {
		joyLogFile = ofstream("sdmc:/logs/joy" + to_string(++logFileIndex) + ".c");
		joyLogFile << "#include \"types.h\"\nstatic const uint16_t step[] = {" << endl;
		startt = clock();
        playMp3("/ftpd/pauseoff.mp3");
	} else {
		joyLogFile << " 0 };" << endl;
		joyLogFile.close();
        playMp3("/ftpd/pauseon.mp3");
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
//		joyLogFile << "//F" << frame++ << endl;
		writeHidEntry();
		if (frame > 1000) {
			// Stop if too long
			setLogging(false);
		}
	}
	//svcSleepThread(3000000L);
}
