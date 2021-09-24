// dllmain.cpp : Defines the entry point for the DLL application.
#include "HooksManager.h"
#include "ChromaImplementation.h"

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    static HooksManager* manager = new HooksManager();
    static ChromaImplementation* chroma = new ChromaImplementation();
    
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        MessageBoxA(NULL, "Apollo Initialized", "SDL2Hook.dll", MB_OK);
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
