#include <switch.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
//#include <libconfig.h++>
//#include <time.h>
#include "log.h"
#include "mp3.h"

using namespace std;

#define MAX_LOG_LENGTH 4000

//Matrix containing the name of each key. Useful for printing when a key is pressed
// char keysNames[32][32] = {
// 	"KEY_A", "KEY_B", "KEY_X", "KEY_Y",
// 	"KEY_LSTICK", "KEY_RSTICK", "KEY_L", "KEY_R",
// 	"KEY_ZL", "KEY_ZR", "KEY_PLUS", "KEY_MINUS",
// 	"KEY_DLEFT", "KEY_DUP", "KEY_DRIGHT", "KEY_DDOWN",
// 	"KEY_LSTICK_LEFT", "KEY_LSTICK_UP", "KEY_LSTICK_RIGHT",
// 	    "KEY_LSTICK_DOWN",
// 	"KEY_RSTICK_LEFT", "KEY_RSTICK_UP", "KEY_RSTICK_RIGHT",
// 	    "KEY_RSTICK_DOWN",
// 	"KEY_SL", "KEY_SR", "KEY_TOUCH", "",
// 	"", "", "", ""
// };

uint16_t keyCodes[16] = {
	0x04,  0x02,  0x08,  0x01, 
	0x400, 0x800, 0x10,  0x20,
	0x40,  0x80,  0x200, 0x100, 
	//Hat
	0x06, 0x00, 0x02, 0x04
};


//int fpsGoal = 30;
//time_t lastTime = time(NULL);
//uint timeFrame = 0;

static long frameLength = 12000000L;

ofstream joyLogFile;
u64 lineNum = 0;
//clock_t startt;

// u32 kDownOld = 0, kHeldOld = 0, kUpOld = 0;	// Previous state
u32 kDown = 0, kHeld = 0, kUp = 0;

int logFileIndex = 0;
bool logging = false;

uint8_t* pseudoL;
uint8_t* pseudoR;
uint8_t* pseudoH;

static uint16_t JOY_CENTER = 0b1000000010000000;

void initLogs(){
	mkdir("sdmc:/joy", 0755);
	mkdir("sdmc:/joy/logs", 0755);
	mkdir("sdmc:/joy/c", 0755);
	//stdout = stderr = fopen("/joy/logs/joylog.log", "w");
}

long read_long (const char* file_name)
{
  FILE* file = fopen (file_name, "r");
  long i = 0;
  fscanf (file, "%ld", &i);    
  fclose (file); 
  return i;       
}

void updateConfig(){
	ofstream settingsLog = ofstream("sdmc:/joy/logs/setting.log");
	settingsLog << "Framelength was previously " << frameLength << endl;

	frameLength = read_long("sdmc:/joy/config_framelength");

	// libconfig::Config cfg;
	// cfg.readFile("sdmc:/joy/config.cfg");
	// fpsGoal = cfg.lookup("fpsGoal");

	settingsLog << "Framelength is now at " << frameLength << endl;
	settingsLog.close();
	return;
}

void closeLogs(){
	joyLogFile.close();
	//fclose(stdout);
}

uint16_t uButtons = 0, uLeft = 0, uRight = 0, uHat = 0x08;
uint16_t uButtonsPrev = 0, uLeftPrev = JOY_CENTER, uRightPrev = JOY_CENTER, uHatPrev = 0x08;
uint16_t repeats = 0;

uint8_t joyScale(int16_t signd){
	//if (signd < -32760) return 0;
	//if (signd > 32760) return 255;
	if (signd == 0) return 128;
	float r = ((float)(signd + 32767) / 65534);
	uint8_t uint8 = r * 255;
	return uint8;
}

void writeHidEntry(){
	hidScanInput();

	kDown = hidKeysDown(CONTROLLER_P1_AUTO);
	kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);
	kUp = hidKeysUp(CONTROLLER_P1_AUTO);

	uButtons = 0;
	uLeft = JOY_CENTER;
	uRight = JOY_CENTER;
	uHat = 0x08;

	int i;
	//Buttons
	for (i = 0; i < 12; i++) {
		if (kHeld & BIT(i)) uButtons |= keyCodes[i];
		//if (kDown & BIT(i)) uButtons |= keyCodes[i];
		//if (kUp & BIT(i)) uButtons ^= keyCodes[i];

		// if (kDown & BIT(i)) (joyLogFile << "//Down " << keysNames[i] << i << endl);
		// if (kHeld & BIT(i)) (joyLogFile << "//Held " << keysNames[i] << i << endl);
		// if (kUp & BIT(i))   (joyLogFile << "//Up " << keysNames[i] << i << endl);
	}
	//Hat
	for (;i < 16; i++) { 
		if (kHeld & BIT(i)) uHat = keyCodes[i];
	}

    JoystickPosition pos_left, pos_right;

    //Read the joysticks' position
    hidJoystickRead(&pos_left, CONTROLLER_P1_AUTO, JOYSTICK_LEFT);
    hidJoystickRead(&pos_right, CONTROLLER_P1_AUTO, JOYSTICK_RIGHT);

    pseudoL = (uint8_t*)(&uLeft);
    pseudoR = (uint8_t*)(&uRight);

    // 16-bit signed integer
    // to
    // 8-bit unsigned integer
    // preserving the ratio

    pseudoL[0] = joyScale(pos_left.dx);
    pseudoL[1] = joyScale(pos_left.dy);
    pseudoR[0] = joyScale(pos_right.dx);
    pseudoR[1] = joyScale(pos_right.dy);

    if (uButtons == uButtonsPrev && uLeft == uLeftPrev && uRight == uRightPrev && uHat == uHatPrev){
    	repeats++;
    } else {
		joyLogFile << "    " << uButtonsPrev << ", " << uLeftPrev << ", " << uRightPrev << ", " << uHatPrev << ", " << repeats << ", " << endl;
    	uButtonsPrev = uButtons;
    	uLeftPrev = uLeft;
    	uRightPrev = uRight;
    	uHatPrev = uHat;
    	repeats = 0;
    	lineNum++;
    }

}

void setLogging(bool newVal){
	logging = newVal;
	// Either we just started, or we're wrapping up. Either way,
	if (logging) {
		joyLogFile = ofstream("sdmc:/joy/c/joy" + to_string(++logFileIndex) + ".c");
		joyLogFile << "#include \"types.h\"\nuint16_t step[] = {" << endl;
		//startt = clock();
        playMp3("/ftpd/pauseoff.mp3");
	} else {
		joyLogFile << " 0 };" << endl;
		joyLogFile << "static int numsteps = " << lineNum << ";" << endl;
		joyLogFile.close();
		fsdevCommitDevice("Sdmc");
        playMp3("/ftpd/pauseon.mp3");
	}
	lineNum = 0;
}

// static long MIN_FRAME_LENGTH = 2500000L;

// void updateTime(){
// 	timeFrame++;

// 	static float fps; 
//     static time_t unixTime = time(NULL);

//     int diff = unixTime - lastTime;
//     if (diff > 0){
//     	fps = timeFrame/diff;

//     	ofstream timeLongStrm = ofstream("sdmc:/joy/logs/timesetting.log");

// 		timeLongStrm << "Time: " << unixTime << endl;
// 		timeLongStrm << "Last Time: " << lastTime << endl;
// 		timeLongStrm << "Diff: " << diff << endl;
// 		timeLongStrm << endl;
// 		timeLongStrm << "FPS: " << fps << endl;
// 		timeLongStrm << "FPS Goal: " << fpsGoal << endl;
// 		timeLongStrm << endl;
// 		timeLongStrm << "Framelength: " << frameLength << endl;
// 		timeLongStrm << "Time frames: " << timeFrame << endl;

//     					   //Should be LONGER when FAST, i.e. when fps-fpsgoal > 0
//     	long newFrameLength = frameLength + (long)((fps-fpsGoal)*10000);

//     	if (newFrameLength < MIN_FRAME_LENGTH) {newFrameLength = MIN_FRAME_LENGTH;}

// 		timeLongStrm << "New FL: " << newFrameLength << endl;
// 		timeLongStrm.close();
// 		fsdevCommitDevice("Sdmc");

//     	frameLength = newFrameLength;
//     	timeFrame = 0;
//     	lastTime = unixTime;
//     }
// }

void inputPoller(){

	hidScanInput();
	u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);

	// Toggle recording with <- + ->
	if ((kHeld & KEY_LSTICK) && (kHeld & KEY_RSTICK)) {
		updateConfig();
		setLogging(!logging);
	}

	if (logging) {
		writeHidEntry();
		if (lineNum > MAX_LOG_LENGTH) {
			// Stop if too long
			setLogging(false);
		}
	}
	//updateTime();

    
    svcSleepThread(frameLength);
}
