#include <switch.h>
#include "SysModule.hpp"
using namespace std;
#include "log.h"
#include "mp3.h"

static long frameLength = 3000L;

int main()
{
    initLogs();

    mp3MutInit();

    while (true)
    {
        inputPoller();
        svcSleepThread(frameLength);
    }

    closeLogs();
    return 0;
}