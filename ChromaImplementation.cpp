#include "ChromaImplementation.h"
#include "FrameController.h"

#ifdef _WIN64
#define CHROMASDKDLL        _T("RzChromaSDK64.dll")
#else
#define CHROMASDKDLL        _T("RzChromaSDK.dll")
#endif
using namespace ChromaSDK;
using namespace ChromaSDK::Keyboard;
using namespace ChromaSDK::Keypad;
using namespace ChromaSDK::Mouse;
using namespace ChromaSDK::Mousepad;
using namespace ChromaSDK::Headset;
using namespace std;

typedef RZRESULT(*INIT)(void);
typedef RZRESULT(*UNINIT)(void);
typedef RZRESULT(*CREATEEFFECT)(RZDEVICEID DeviceId, ChromaSDK::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
typedef RZRESULT(*CREATEKEYBOARDEFFECT)(ChromaSDK::Keyboard::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
typedef RZRESULT(*CREATEHEADSETEFFECT)(ChromaSDK::Headset::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
typedef RZRESULT(*CREATEMOUSEPADEFFECT)(ChromaSDK::Mousepad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
typedef RZRESULT(*CREATEMOUSEEFFECT)(ChromaSDK::Mouse::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
typedef RZRESULT(*CREATEKEYPADEFFECT)(ChromaSDK::Keypad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
typedef RZRESULT(*SETEFFECT)(RZEFFECTID EffectId);
typedef RZRESULT(*DELETEEFFECT)(RZEFFECTID EffectId);
typedef RZRESULT(*REGISTEREVENTNOTIFICATION)(HWND hWnd);
typedef RZRESULT(*UNREGISTEREVENTNOTIFICATION)(void);
typedef RZRESULT(*QUERYDEVICE)(RZDEVICEID DeviceId, ChromaSDK::DEVICE_INFO_TYPE& DeviceInfo);

INIT Init = nullptr;
UNINIT UnInit = nullptr;
CREATEEFFECT CreateEffect = nullptr;
CREATEKEYBOARDEFFECT CreateKeyboardEffect = nullptr;
CREATEMOUSEEFFECT CreateMouseEffect = nullptr;
CREATEHEADSETEFFECT CreateHeadsetEffect = nullptr;
CREATEMOUSEPADEFFECT CreateMousepadEffect = nullptr;
CREATEKEYPADEFFECT CreateKeypadEffect = nullptr;
SETEFFECT SetEffect = nullptr;
DELETEEFFECT DeleteEffect = nullptr;
QUERYDEVICE QueryDevice = nullptr;

#define MAX_EFFECTS     100

typedef struct _EFFECTDATATYPE
{
	LONG numEffects;
	BOOL repeat;
	HANDLE thread;
	struct _EFFECT
	{
		RZEFFECTID id;
		LONG delay;
	} Effect[MAX_EFFECTS];
} EFFECTDATATYPE;

struct GUIDCompare
{
	bool operator()(const GUID& Left, const GUID& Right) const
	{
		return memcmp(&Left, &Right, sizeof(Right)) < 0;
	}
};

std::map<RZEFFECTID, EFFECTDATATYPE, GUIDCompare> g_Effects;

DWORD WINAPI Thread_RenderEffects(LPVOID lpParameter)
{
	RZEFFECTID* pEffectId = (RZEFFECTID*)lpParameter;

	auto iterator = g_Effects.find(*pEffectId);
	if (iterator != g_Effects.end())
	{
		EFFECTDATATYPE* pEffectData = &iterator->second;

		CFrameController FrameControl(30);

		if (pEffectData->repeat == FALSE)
		{
			for (int i = 0; i < pEffectData->numEffects; i++)
			{
				FrameControl.Begin();

				SetEffect(pEffectData->Effect[i].id);

				Sleep(pEffectData->Effect[i].delay);

				FrameControl.End();
			}
		}
		else
		{
			while (pEffectData->repeat)
			{
				for (int i = 0; i < pEffectData->numEffects; i++)
				{
					FrameControl.Begin();

					SetEffect(pEffectData->Effect[i].id);

					Sleep(pEffectData->Effect[i].delay);

					FrameControl.End();
				}
			};
		}
	}

	return 0;
}

ChromaImplementation::ChromaImplementation() :m_ChromaSDKModule(nullptr)
{
}
ChromaImplementation::~ChromaImplementation()
{
}

BOOL ChromaImplementation::Initialize()
{
	if (m_ChromaSDKModule == nullptr)
	{
		m_ChromaSDKModule = LoadLibrary(CHROMASDKDLL);
		if (m_ChromaSDKModule == nullptr)
		{
			return FALSE;
		}
	}

	if (Init == nullptr)
	{
		auto Result = RZRESULT_INVALID;
		Init = reinterpret_cast<INIT>(GetProcAddress(m_ChromaSDKModule, "Init"));
		if (Init)
		{
			Result = Init();
			if (Result == RZRESULT_SUCCESS)
			{
				CreateEffect = reinterpret_cast<CREATEEFFECT>(GetProcAddress(m_ChromaSDKModule, "CreateEffect"));
				CreateKeyboardEffect = reinterpret_cast<CREATEKEYBOARDEFFECT>(GetProcAddress(m_ChromaSDKModule, "CreateKeyboardEffect"));
				CreateMouseEffect = reinterpret_cast<CREATEMOUSEEFFECT>(GetProcAddress(m_ChromaSDKModule, "CreateMouseEffect"));
				CreateHeadsetEffect = reinterpret_cast<CREATEHEADSETEFFECT>(GetProcAddress(m_ChromaSDKModule, "CreateHeadsetEffect"));
				CreateMousepadEffect = reinterpret_cast<CREATEMOUSEPADEFFECT>(GetProcAddress(m_ChromaSDKModule, "CreateMousepadEffect"));
				CreateKeypadEffect = reinterpret_cast<CREATEKEYPADEFFECT>(GetProcAddress(m_ChromaSDKModule, "CreateKeypadEffect"));
				SetEffect = reinterpret_cast<SETEFFECT>(GetProcAddress(m_ChromaSDKModule, "SetEffect"));
				DeleteEffect = reinterpret_cast<DELETEEFFECT>(GetProcAddress(m_ChromaSDKModule, "DeleteEffect"));
				QueryDevice = reinterpret_cast<QUERYDEVICE>(GetProcAddress(m_ChromaSDKModule, "QueryDevice"));

				if (CreateEffect &&
					CreateKeyboardEffect &&
					CreateMouseEffect &&
					CreateHeadsetEffect &&
					CreateMousepadEffect &&
					CreateKeypadEffect &&
					SetEffect &&
					DeleteEffect &&
					QueryDevice)
				{
					return TRUE;
				}
				else
				{
					return FALSE;
				}
			}
		}
	}

	return TRUE;
}

void ChromaImplementation::ResetEffects(size_t DeviceType)
{
	switch (DeviceType)
	{
	case 0:
		if (CreateKeyboardEffect)
		{
			CreateKeyboardEffect(ChromaSDK::Keyboard::CHROMA_NONE, nullptr, nullptr);
		}

		if (CreateMousepadEffect)
		{
			CreateMousepadEffect(ChromaSDK::Mousepad::CHROMA_NONE, nullptr, nullptr);
		}

		if (CreateMouseEffect)
		{
			CreateMouseEffect(ChromaSDK::Mouse::CHROMA_NONE, nullptr, nullptr);
		}

		if (CreateHeadsetEffect)
		{
			CreateHeadsetEffect(ChromaSDK::Headset::CHROMA_NONE, nullptr, nullptr);
		}

		if (CreateKeypadEffect)
		{
			CreateKeypadEffect(ChromaSDK::Keypad::CHROMA_NONE, nullptr, nullptr);
		}
		break;
	case 1:
		if (CreateKeyboardEffect)
		{
			CreateKeyboardEffect(ChromaSDK::Keyboard::CHROMA_NONE, nullptr, nullptr);
		}
		break;
	case 2:
		if (CreateMousepadEffect)
		{
			CreateMousepadEffect(ChromaSDK::Mousepad::CHROMA_NONE, nullptr, nullptr);
		}
		break;
	case 3:
		if (CreateMouseEffect)
		{
			CreateMouseEffect(ChromaSDK::Mouse::CHROMA_NONE, nullptr, nullptr);
		}
		break;
	case 4:
		if (CreateHeadsetEffect)
		{
			CreateHeadsetEffect(ChromaSDK::Headset::CHROMA_NONE, nullptr, nullptr);
		}
		break;
	case 5:
		if (CreateKeypadEffect)
		{
			CreateKeypadEffect(ChromaSDK::Keypad::CHROMA_NONE, nullptr, nullptr);
		}
		break;
	}
}

void ChromaImplementation::CreateKeyboardEffectImpl(ChromaSDK::Keyboard::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId)
{
	if (CreateKeyboardEffect == NULL) return;

	CreateKeyboardEffect(Effect, pParam, pEffectId);
}

void ChromaImplementation::SetEffectImpl(RZEFFECTID EffectId)
{
	auto iterator = g_Effects.find(EffectId);
	if (iterator != g_Effects.end())
	{
		if (iterator->second.repeat == FALSE)
		{
			HANDLE hThread = CreateThread(NULL, 0, Thread_RenderEffects, (LPVOID)&iterator->first, 0, NULL);
			if (hThread != NULL)
			{
				CloseHandle(hThread);
			}
		}
		else
		{
			HANDLE hThread = CreateThread(NULL, 0, Thread_RenderEffects, (LPVOID)&iterator->first, 0, NULL);
			if (hThread != NULL)
			{
				iterator->second.thread = hThread;
			}
		}
	}
	else
	{
		if (SetEffect == NULL) return;

		SetEffect(EffectId);
	}
}

void ChromaImplementation::CreateEffectGroup(RZEFFECTID* pGroupEffectId, BOOL Repeat)
{
	RZEFFECTID EffectId = GUID_NULL;
	if (SUCCEEDED(::CoCreateGuid(&EffectId)))
	{
		EFFECTDATATYPE EffectData = {};

		EffectData.numEffects = 0;
		EffectData.repeat = Repeat;

		g_Effects.insert(make_pair(EffectId, EffectData));

		*pGroupEffectId = EffectId;
	}
}

void ChromaImplementation::AddToGroup(RZEFFECTID GroupEffectId, RZEFFECTID EffectId, LONG DelayMS)
{
	auto iterator = g_Effects.find(GroupEffectId);
	if (iterator != g_Effects.end())
	{
		LONG lIndex = iterator->second.numEffects;

		iterator->second.Effect[lIndex].id = EffectId;
		iterator->second.Effect[lIndex].delay = DelayMS;

		iterator->second.numEffects++;
	}
}