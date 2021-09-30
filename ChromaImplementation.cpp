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

#define EVENT_NAME  _T("{4784D90A-1179-4F7D-8558-52511D809190}")

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

ChromaImplementation::ChromaImplementation() :
	m_ChromaSDKModule(NULL),
	m_ChromaSDKEvent(NULL)
{
}

ChromaImplementation::~ChromaImplementation()
{
}

BOOL ChromaImplementation::Initialize()
{
	if (m_ChromaSDKModule == NULL)
	{
		m_ChromaSDKModule = ::LoadLibrary(CHROMASDKDLL);
		if (m_ChromaSDKModule != NULL)
		{
			INIT Init = (INIT)::GetProcAddress(m_ChromaSDKModule, "Init");
			if (Init != NULL)
			{
				RZRESULT rzResult = Init();
				if (rzResult == RZRESULT_SUCCESS)
				{
					CreateEffect = (CREATEEFFECT)::GetProcAddress(m_ChromaSDKModule, "CreateEffect");
					CreateKeyboardEffect = (CREATEKEYBOARDEFFECT)::GetProcAddress(m_ChromaSDKModule, "CreateKeyboardEffect");
					CreateMouseEffect = (CREATEMOUSEEFFECT)::GetProcAddress(m_ChromaSDKModule, "CreateMouseEffect");
					CreateMousepadEffect = (CREATEMOUSEPADEFFECT)::GetProcAddress(m_ChromaSDKModule, "CreateMousepadEffect");
					CreateKeypadEffect = (CREATEKEYPADEFFECT)::GetProcAddress(m_ChromaSDKModule, "CreateKeypadEffect");
					CreateHeadsetEffect = (CREATEHEADSETEFFECT)::GetProcAddress(m_ChromaSDKModule, "CreateHeadsetEffect");
					SetEffect = (SETEFFECT)GetProcAddress(m_ChromaSDKModule, "SetEffect");
					DeleteEffect = (DELETEEFFECT)GetProcAddress(m_ChromaSDKModule, "DeleteEffect");

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
						if (m_ChromaSDKEvent == NULL)
						{
							m_ChromaSDKEvent = ::CreateEvent(NULL, TRUE, FALSE, EVENT_NAME);
						}

						return TRUE;

					}
					else
					{
						return FALSE;
					}	
				}
			}
		}
	}

	return TRUE;

}

void ChromaImplementation::UnInitialize()
{
	// Free memeory
	while (!g_Effects.empty())
	{
		auto iterator = g_Effects.begin();
		for (int i = 0; i < iterator->second.numEffects; i++)
		{
			DeleteEffect(iterator->second.Effect[i].id);
		}

		g_Effects.erase(iterator);
	};

	if (m_ChromaSDKEvent != NULL)
	{
		::CloseHandle(m_ChromaSDKEvent);
		m_ChromaSDKEvent = NULL;
	}

	if (m_ChromaSDKModule != NULL)
	{
		UNINIT UnInit = (UNINIT)::GetProcAddress(m_ChromaSDKModule, "UnInit");
		if (UnInit != NULL)
		{
			RZRESULT rzResult = UnInit();
			if (rzResult != RZRESULT_SUCCESS)
			{
				// Some error here
			}
		}

		::FreeLibrary(m_ChromaSDKModule);
		m_ChromaSDKModule = NULL;
	}
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

void ChromaImplementation::DeleteEffectImpl(RZEFFECTID EffectId)
{
	auto iterator = g_Effects.find(EffectId);
	if (iterator != g_Effects.end())
	{
		EFFECTDATATYPE EffectData = iterator->second;
		for (int i = 0; i < EffectData.numEffects; i++)
		{
			DeleteEffect(EffectData.Effect[i].id);
		}

		g_Effects.erase(iterator);
	}
	else
	{
		if (DeleteEffect == NULL) return;

		DeleteEffect(EffectId);
	}
}

void ChromaImplementation::StopEffectImpl(RZEFFECTID EffectId)
{
	auto iterator = g_Effects.find(EffectId);
	if (iterator != g_Effects.end())
	{
		if ((iterator->second.repeat == TRUE) &&
			(iterator->second.thread != NULL))
		{
			iterator->second.repeat = FALSE;

			CloseHandle(iterator->second.thread);

			iterator->second.thread = NULL;
		}
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