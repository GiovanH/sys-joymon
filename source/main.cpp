#include <switch.h>
#include "SysModule.hpp"
#include "log.h"
#include "mp3.h"
using namespace std;

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
