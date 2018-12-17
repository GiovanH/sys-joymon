#include <switch.h>
#include "SysModule.hpp"
using namespace std;
#include "log.h"
#include "mp3.h"
#include <time.h>

int main()
{
    initLogs();

    mp3MutInit();

    while (true)
    {
        inputPoller();
    }

    closeLogs();
    return 0;
}
