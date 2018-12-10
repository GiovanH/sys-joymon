#include <switch.h>
#include "SysModule.hpp"
using namespace std;
#include "log.h"

static long frameLength = 3000L;

int main()
{
    initLogs();

    while (true)
    {
        inputPoller();
        svcSleepThread(frameLength);
    }

    closeLogs();
    return 0;
}