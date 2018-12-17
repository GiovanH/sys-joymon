#include <switch.h>
// Include the most common headers from the C standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using namespace std;

extern "C"
{
    // Sysmodules should not use applet*.
    u32 __nx_applet_type = AppletType_None;

    // Adjust size as needed.
    //#define KIP_HEAP 0x80000
    size_t nx_inner_heap_size = KIP_HEAP;
    char   nx_inner_heap[KIP_HEAP];

    void __libnx_initheap(void)
    {
        void*  addr = nx_inner_heap;
        size_t size = nx_inner_heap_size;

        // Newlib
        extern char* fake_heap_start;
        extern char* fake_heap_end;

        fake_heap_start = (char*)addr;
        fake_heap_end   = (char*)addr + size;
    }

    // Init/exit services, update as needed.
    void __attribute__((weak)) __appInit(void)
    {
        Result rc;

        // Initialize default services.
        rc = smInitialize();
        if (R_FAILED(rc))
            fatalSimple(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));

        // Enable this if you want to use HID.
        rc = hidInitialize();
        if (R_FAILED(rc))
            fatalSimple(MAKERESULT(Module_Libnx, LibnxError_InitFail_HID));

        //Enable this if you want to use time.
        rc = timeInitialize();
        if (R_FAILED(rc))
            fatalSimple(MAKERESULT(Module_Libnx, LibnxError_InitFail_Time));

        rc = fsInitialize();
        if (R_FAILED(rc))
            fatalSimple(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));

        fsdevMountSdmc();
    }

    void __attribute__((weak)) userAppExit(void);

    void __attribute__((weak)) __appExit(void)
    {
        // Cleanup default services.
        fsdevUnmountAll();
        fsExit();
        timeExit(); //Enable this if you want to use time.
        hidExit(); // Enable this if you want to use HID.
        smExit();
    }
}