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

    if (rgb == "0000FF") {
        // this is returned after getting loot or after any loading screen
    }
    else if (rgb=="FFFFFF") {
        // this is returned for hammers
    }
    else {
        // anything else return values of Loot.Color from "C:\Program Files\Steam\steamapps\common\Hades\Content\Scripts\LootData.lua"
		// however color from LightingColor is more accurate/suitable for Chroma usage
        static RZEFFECTID EffectId = GUID_NULL;
        if (IsEqualGUID(EffectId, GUID_NULL))
        {
            impl.CreateEffectGroup(&EffectId);

            RZEFFECTID Frame1;
            RZEFFECTID Frame2;

            ChromaSDK::Keyboard::STATIC_EFFECT_TYPE Static = {};
            Static.Color = GREEN;

            impl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_STATIC, &Static, &Frame1);

            impl.CreateKeyboardEffectImpl(ChromaSDK::Keyboard::CHROMA_NONE, NULL, &Frame2);

            impl.AddToGroup(EffectId, Frame2, 200);  // Clear the LEDs

            // Blink 3 times
            impl.AddToGroup(EffectId, Frame1, 200);
            impl.AddToGroup(EffectId, Frame2, 200);
            impl.AddToGroup(EffectId, Frame1, 200);
            impl.AddToGroup(EffectId, Frame2, 200);
            impl.AddToGroup(EffectId, Frame1, 200);
            impl.AddToGroup(EffectId, Frame2, 200);
        }

        impl.SetEffectImpl(EffectId);
    }

    return (void* (*)(void* ctrl, void* red, void* green, void* blue)) HooksManager::GetOriginalFunction((ULONG_PTR)HookSDL_JoystickSetLED);
}