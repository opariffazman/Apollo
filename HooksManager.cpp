#include "HooksManager.h"
#include "ChromaImplementation.h"

bool HooksManager::isInitialized = false;
ChromaImplementation impl;
auto init = impl.Initialize();

BOOL(__cdecl* HooksManager::HookFunction)(ULONG_PTR OriginalFunction, ULONG_PTR NewFunction);
VOID(__cdecl* HooksManager::UnhookFunction)(ULONG_PTR Function);
ULONG_PTR(__cdecl* HooksManager::GetOriginalFunction)(ULONG_PTR Hook);
void* HookSDL_JoystickSetLED(void* joystickPtr, void* red, void* green, void* blue);


HooksManager::HooksManager()
{
    if (!isInitialized)
    {
        if (HookFunction == NULL || UnhookFunction == NULL || GetOriginalFunction == NULL)
        {
            HMODULE hHookEngineDll = LoadLibrary(L"NtHookEngine.dll");

            HookFunction = (BOOL(__cdecl*)(ULONG_PTR, ULONG_PTR)) GetProcAddress(hHookEngineDll, "HookFunction");
            UnhookFunction = (VOID(__cdecl*)(ULONG_PTR)) GetProcAddress(hHookEngineDll, "UnhookFunction");
            GetOriginalFunction = (ULONG_PTR(__cdecl*)(ULONG_PTR)) GetProcAddress(hHookEngineDll, "GetOriginalFunction");
        }

        isInitialized = true;

        hookFunctions();
    }
}

HooksManager::~HooksManager()
{
    removeHooks();
}

void HooksManager::hookFunctions() {
    if (HookFunction == NULL || UnhookFunction == NULL || GetOriginalFunction == NULL)
        return;

    hLibrary = LoadLibrary(L"SDL2.dll");
    if (hLibrary == NULL) {
        return;
    }

    HookFunction((ULONG_PTR)GetProcAddress(hLibrary, "SDL_JoystickSetLED"), (ULONG_PTR)HookSDL_JoystickSetLED);


}
void HooksManager::removeHooks()
{
    if (HookFunction != NULL && UnhookFunction != NULL && GetOriginalFunction != NULL && hLibrary != NULL)
    {
        UnhookFunction((ULONG_PTR)GetProcAddress(hLibrary, "SDL_JoystickSetLED"));
    }
}

void* HookSDL_JoystickSetLED(void* joystickPtr, void* red, void* green, void* blue) {

    std::ostringstream r, g, b;

    r << red;
    g << green;
    b << blue;

    std::string rraw = r.str();
    std::string graw = g.str();
    std::string braw = b.str();

    std::string rgb = rraw.substr(rraw.length() - 2) + graw.substr(graw.length() - 2) + braw.substr(braw.length() - 2);


    if (rgb != "0000FF" && rgb != "FFFFFF") {
        // anything else return values of Loot.Color from "C:\Program Files\Steam\steamapps\common\Hades\Content\Scripts\LootData.lua"
		// however color from LightingColor is more accurate/suitable for Chroma usage
        static RZEFFECTID EffectId = GUID_NULL;
        ChromaSDK::Keyboard::STATIC_EFFECT_TYPE Static = {};

        if (rgb == "FFFF40") Static.Color = ZEUS;
        else if (rgb == "FF1400") Static.Color = ARES;
        else if (rgb == "6EFF00") Static.Color = ARTEMIS;
        else if (rgb == "FF32F0") Static.Color = APHRODITE;
        else if (rgb == "6040FF") Static.Color = ATHENA;
        else if (rgb == "00C8FF") Static.Color = POSEIDON;
        else if (rgb == "60BDFF") Static.Color = DEMETER;
        else if (rgb == "FF5A00") Static.Color = HERMES;
        else if (rgb == "6419FF") Static.Color = CHAOS;
        else Static.Color = WHITE;

        impl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_STATIC, &Static, NULL);
        impl.SetEffectImpl(EffectId);
    }
    else {
        static RZEFFECTID EffectId = GUID_NULL;

        impl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_NONE, NULL);
        impl.SetEffectImpl(EffectId);
    }

    //MessageBoxA(NULL, rgb.c_str(), "ApolloHook.dll", MB_OK);

    return (void* (*)(void* ctrl, void* red, void* green, void* blue)) HooksManager::GetOriginalFunction((ULONG_PTR)HookSDL_JoystickSetLED);
}