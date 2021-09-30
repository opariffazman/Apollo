#pragma once

#include <tchar.h>
#include <assert.h>
#include <wtypes.h>
#include <cguid.h>
#include <map>

#include "stdafx.h"
#include "RzChromaSDKDefines.h"
#include "RzChromaSDKTypes.h"
#include "RzErrors.h"

#ifndef _CHROMASDKIMPL_H_
#define _CHROMASDKIMPL_H_


#pragma once

#ifndef DLL_COMPILED
#define DLL_INTERNAL __declspec( dllexport )
#endif 


// Define all Colours you want
const COLORREF BLACK = RGB(0, 0, 0);
const COLORREF WHITE = RGB(255, 255, 255);
const COLORREF RED = RGB(255, 0, 0);
const COLORREF GREEN = RGB(0, 255, 0);
const COLORREF BLUE = RGB(0, 0, 255);
const COLORREF YELLOW = RGB(255, 255, 0);
const COLORREF PURPLE = RGB(128, 0, 128);
const COLORREF CYAN = RGB(00, 255, 255);
const COLORREF ORANGE = RGB(255, 165, 00);
const COLORREF PINK = RGB(255, 192, 203);
const COLORREF GREY = RGB(125, 125, 125);

const COLORREF ZEUS = RGB(235, 206, 87);
const COLORREF ARES = RGB(255, 94, 73);
const COLORREF ARTEMIS = RGB(210, 255, 97);
const COLORREF APHRODITE = RGB(255, 196, 240);
const COLORREF ATHENA = RGB(253, 189, 49);
const COLORREF POSEIDON = RGB(139, 199, 253);
const COLORREF DEMETER = RGB(253, 189, 49);
const COLORREF HERMES = RGB(255, 120, 0);
const COLORREF CHAOS = RGB(100, 25, 255);
//You dont have to define your colors as COLORREFs, just use the RGB(xxx,xxx,xxx) function like above

#define ALL_DEVICES         0
#define KEYBOARD_DEVICES    1
#define MOUSEMAT_DEVICES    2
#define MOUSE_DEVICES       3
#define HEADSET_DEVICES     4
#define KEYPAD_DEVICES      5

//Class of your Chroma Implementation
class ChromaImplementation
{
public:
	ChromaImplementation();
	~ChromaImplementation();
	//BOOL Initialize();
	BOOL Initialize();
	void UnInitialize();

	void ResetEffects(size_t DeviceType);
	//Define your methods here
	void CreateKeyboardEffectImpl(ChromaSDK::Keyboard::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId = NULL);
	void SetEffectImpl(RZEFFECTID EffectId);
	void CreateEffectGroup(RZEFFECTID* pGroupEffectId, BOOL Repeat = FALSE);
	void AddToGroup(RZEFFECTID GroupEffectId, RZEFFECTID EffectId, LONG DelayMS = 100);
	void DeleteEffectImpl(RZEFFECTID EffectId);
	void StopEffectImpl(RZEFFECTID EffectId);

	BOOL IsDeviceConnected(RZDEVICEID DeviceId);

private:
	HMODULE m_ChromaSDKModule;
	HANDLE m_ChromaSDKEvent;

};

extern ChromaImplementation g_ChromaImplementation;

#endif
