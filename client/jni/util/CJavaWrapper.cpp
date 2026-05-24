#include "CJavaWrapper.h"
#include "../main.h"

extern "C" JavaVM* javaVM;

#include "..//keyboard.h"
#include "..//chatwindow.h"
#include "..//CSettings.h"
#include "../net/netgame.h"
#include "../game/game.h"
#include "../str_obfuscator_no_template.hpp"
#include "../dialog.h"
#include "..//scoreboard.h"

extern CScoreBoard* pScoreBoard;
extern CKeyBoard* pKeyBoard;
extern CChatWindow* pChatWindow;
extern CSettings* pSettings;
extern CNetGame* pNetGame;
extern CGame* pGame;
extern CDialogWindow *pDialogWindow;

JNIEnv* CJavaWrapper::GetEnv()
{
	JNIEnv* env = nullptr;
	int getEnvStat = javaVM->GetEnv((void**)& env, JNI_VERSION_1_4);

	if (getEnvStat == JNI_EDETACHED)
	{
		Log(OBFUSCATE("GetEnv: not attached"));
		if (javaVM->AttachCurrentThread(&env, NULL) != 0)
		{
			Log(OBFUSCATE("Failed to attach"));
			return nullptr;
		}
	}
	if (getEnvStat == JNI_EVERSION)
	{
		Log(OBFUSCATE("GetEnv: version not supported"));
		return nullptr;
	}

	if (getEnvStat == JNI_ERR)
	{
		Log(OBFUSCATE("GetEnv: JNI_ERR"));
		return nullptr;
	}

	return env;
}



void CJavaWrapper::FinishActivity()
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(activity, s_FinishActivity);

	EXCEPTION_CHECK(env);
}

std::string CJavaWrapper::GetClipboardString()
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log(OBFUSCATE("No env"));
		return std::string("");
	}

	auto retn = (jbyteArray)env->CallObjectMethod(activity, s_GetClipboardText);

	if ((env)->ExceptionCheck())
	{
		(env)->ExceptionDescribe();
		(env)->ExceptionClear();
		return std::string("");
	}

	if (!retn)
	{
		return std::string("");
	}

	jboolean isCopy = true;

	jbyte* pText = env->GetByteArrayElements(retn, &isCopy);
	jsize length = env->GetArrayLength(retn);

	std::string str((char*)pText, length);

	env->ReleaseByteArrayElements(retn, pText, JNI_ABORT);
	
	return str;
}

void CJavaWrapper::ShowInputLayout()
{
	// --- FIX: CHẶN BÀN PHÍM ANDROID NẾU ĐANG DÙNG BÀN PHÍM CUSTOM ---
	if (pKeyBoard && pKeyBoard->IsOpen())
	{
		Log(OBFUSCATE("Custom keyboard is open, blocking system keyboard."));
		return;
	}
	// -----------------------------------------------------------------

	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log(OBFUSCATE("No env"));
		return;
	}

	env->CallVoidMethod(activity, s_ShowInputLayout);

	EXCEPTION_CHECK(env);
}


void CJavaWrapper::ShowDonate(int money, int bc)
{
    JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}
    env->CallVoidMethod(this->activity, this->s_showDonate, money, bc);
}
void CJavaWrapper::UpdateDonate(int money, int bc)
{
    JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}
    env->CallVoidMethod(this->activity, this->s_updateDonate, money, bc);
}
void CJavaWrapper::show_sc(int money, int bc)
{
    JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}
    env->CallVoidMethod(this->activity, this->s_show_sc, money, bc);
}


void CJavaWrapper::HideInputLayout()
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log(OBFUSCATE("No env"));
		return;
	}

	env->CallVoidMethod(activity, s_HideInputLayout);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::ShowClientSettings()
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log(OBFUSCATE("No env"));
		return;
	}

	env->CallVoidMethod(activity, s_ShowClientSettings);

	EXCEPTION_CHECK(env);
}

void CJavaWrapper::MakeDialog(int dialogId, int dialogTypeId, char* caption, char* content, char* leftBtnText, char* rightBtnText)
{
    JNIEnv* env = GetEnv();
    if (!env)
    {
	Log("No env");
	return;
    }
    jclass strClass = env->FindClass("java/lang/String");
    jmethodID ctorID = env->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    jstring encoding = env->NewStringUTF("UTF-8");
    jbyteArray bytes = env->NewByteArray(strlen(caption));
    env->SetByteArrayRegion(bytes, 0, strlen(caption), (jbyte*)caption);
    jstring str1 = (jstring)env->NewObject(strClass, ctorID, bytes, encoding);
    //
    jclass strClass1 = env->FindClass("java/lang/String");
    jmethodID ctorID1 = env->GetMethodID(strClass1, "<init>", "([BLjava/lang/String;)V");
    jstring encoding1 = env->NewStringUTF("UTF-8");
    jbyteArray bytes1 = env->NewByteArray(strlen(content));
    env->SetByteArrayRegion(bytes1, 0, strlen(content), (jbyte*)content);
    jstring str2 = (jstring)env->NewObject(strClass1, ctorID1, bytes1, encoding1);
    //
    jclass strClass2 = env->FindClass("java/lang/String");
    jmethodID ctorID2 = env->GetMethodID(strClass2, "<init>", "([BLjava/lang/String;)V");
    jstring encoding2 = env->NewStringUTF("UTF-8");
    jbyteArray bytes2 = env->NewByteArray(strlen(leftBtnText));
    env->SetByteArrayRegion(bytes2, 0, strlen(leftBtnText), (jbyte*)leftBtnText);
    jstring str3 = (jstring)env->NewObject(strClass2, ctorID2, bytes2, encoding2);
    //
    jclass strClass3 = env->FindClass("java/lang/String");
    jmethodID ctorID3 = env->GetMethodID(strClass3, "<init>", "([BLjava/lang/String;)V");
    jstring encoding3 = env->NewStringUTF("UTF-8");
    jbyteArray bytes3 = env->NewByteArray(strlen(rightBtnText));
    env->SetByteArrayRegion(bytes3, 0, strlen(rightBtnText), (jbyte*)rightBtnText);
    jstring str4 = (jstring)env->NewObject(strClass3, ctorID3, bytes3, encoding3);

    env->CallVoidMethod(activity, s_MakeDialog, dialogId, dialogTypeId, str1, str2, str3, str4);

    EXCEPTION_CHECK(env);
}

void CJavaWrapper::SetUseFullScreen(int b)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log(OBFUSCATE("No env"));
		return;
	}

	env->CallVoidMethod(activity, s_SetUseFullScreen, b);

	EXCEPTION_CHECK(env);
}

extern int g_iStatusDriftChanged;
#include "..//CDebugInfo.h"
extern "C"
{
	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_onInputEnd(JNIEnv* pEnv, jobject thiz, jbyteArray str)
	{
		if (pKeyBoard)
		{
			pKeyBoard->OnNewKeyboardInput(pEnv, thiz, str);
		}
	}
	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_sendDialogResponse(JNIEnv* pEnv, jobject thiz, jint i3, jint i, jint i2, jbyteArray str)
	{
		jboolean isCopy = true;

		jbyte* pMsg = pEnv->GetByteArrayElements(str, &isCopy);
		jsize length = pEnv->GetArrayLength(str);

		std::string szStr((char*)pMsg, length);

		if(pNetGame) {
			pNetGame->SendDialogResponse(i, i3, i2, (char*)szStr.c_str());
			pGame->FindPlayerPed()->TogglePlayerControllable(true);
		}

		pEnv->ReleaseByteArrayElements(str, pMsg, JNI_ABORT);
	}
	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_togglePlayer(JNIEnv* pEnv, jobject thiz, jint toggle) {
		if(toggle)
			pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed()->TogglePlayerControllable(false);
		else
			pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed()->TogglePlayerControllable(true);
	}
	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_onEventBackPressed(JNIEnv* pEnv, jobject thiz)
	{
		if (pKeyBoard)
		{
			if (pKeyBoard->IsOpen())
			{
				Log(OBFUSCATE("Closing keyboard"));
				pKeyBoard->Close();
			}
		}
	}
	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_onNativeHeightChanged(JNIEnv* pEnv, jobject thiz, jint orientation, jint height)
	{
		if (pChatWindow)
		{
			pChatWindow->SetLowerBound(height);
		}
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeCutoutSettings(JNIEnv* pEnv, jobject thiz, jboolean b)
	{
		if (pSettings)
		{
			pSettings->GetWrite().iCutout = b;
		}
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeFpsCounterSettings(JNIEnv* pEnv, jobject thiz, jboolean b)
	{
		if (pSettings)
		{
			pSettings->GetWrite().iFPSCounter = b;
		}

		CDebugInfo::SetDrawFPS(b);
	}
	
	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeDialog(JNIEnv* pEnv, jobject thiz, jboolean b)
	{
		if (pSettings)
		{
			pSettings->GetWrite().iDialog = b;
		}
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeNameTag(JNIEnv* pEnv, jobject thiz, jboolean b)
	{
		if (pSettings)
			pSettings->GetWrite().iNameTag = b;
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNative3DText(JNIEnv* pEnv, jobject thiz, jboolean b)
	{
		if (pSettings)
			pSettings->GetWrite().i3DText = b;
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeSkyBox(JNIEnv* pEnv, jobject thiz, jboolean b)
	{
		if (pSettings)
			pSettings->GetWrite().iSkyBox = b;
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeFixedTextDraw(JNIEnv* pEnv, jobject thiz, jboolean b)
	{
		if (pSettings)
			pSettings->GetWrite().iTextDraw = b;
	}

	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeFixedTextDraw(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
			return pSettings->GetReadOnly().iTextDraw;

		return 0;
	}

	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeSkyBox(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
			return pSettings->GetReadOnly().iSkyBox;

		return 0;
	}

	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeCutoutSettings(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
		{
			return pSettings->GetReadOnly().iCutout;
		}
		return 0;
	}
	
	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeDialog(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
		{
			return pSettings->GetReadOnly().iDialog;
		}
		return 0;
	}
	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeHud(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
		{
			return pSettings->GetReadOnly().iHud;
		}
		return 0;
	}

	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeKeyboardSettings(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
		{
			return pSettings->GetReadOnly().iAndroidKeyboard;
		}
		return 0;
	}

	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeFpsCounterSettings(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
		{
			return pSettings->GetReadOnly().iFPSCounter;
		}
		return 0;
	}

	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeHpArmourText(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
		{
			return pSettings->GetReadOnly().iHPArmourText;
		}
		return 0;
	}

	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeOutfitGunsSettings(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
		{
			return pSettings->GetReadOnly().iOutfitGuns;
		}
		return 0;
	}

	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativePcMoney(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
		{
			return pSettings->GetReadOnly().iPCMoney;
		}
		return 0;
	}

	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeRadarrect(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
		{
			return pSettings->GetReadOnly().iRadarRect;
		}
		return 0;
	}

	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNative3DText(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
			return pSettings->GetReadOnly().i3DText;

		return 0;
	}

	JNIEXPORT jboolean JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeNameTag(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
			return pSettings->GetReadOnly().iNameTag;

		return 0;
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_onSettingsWindowSave(JNIEnv* pEnv, jobject thiz)
	{
		if (pSettings)
		{
			pSettings->Save();
		}
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_onSettingsWindowDefaults(JNIEnv* pEnv, jobject thiz, jint category)
	{
		if (pSettings)
		{
			pSettings->ToDefaults(category);
			if (pChatWindow)
			{
				pChatWindow->m_bPendingReInit = true;
			}
		}
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_sendDonateClick(JNIEnv* pEnv, jobject thiz, jint id)
	{
        switch (id) {
        	case 0: pNetGame->SendChatCommand("/openbc"); break;
            case 1: pNetGame->SendChatCommand("/openrub"); break;
			case 2: pNetGame->SendChatCommand("/keys"); break;
			case 3: pNetGame->SendChatCommand("/bp"); break;
			case 4: pNetGame->SendChatCommand("/nabori"); break;
			case 5: pNetGame->SendChatCommand("/akss"); break;
			case 6: pNetGame->SendChatCommand("/changename"); break;
			case 7: pNetGame->SendChatCommand("/unnwarn"); break;
			case 8: pNetGame->SendChatCommand("/alllic"); break;
			case 9: pNetGame->SendChatCommand("/powerbuy"); break;
			case 10: pNetGame->SendChatCommand("/bomboxbuy"); break;
			case 11: pNetGame->SendChatCommand("/buyslot"); break;
			case 12: pNetGame->SendChatCommand("/buynomber"); break;
			case 13: pNetGame->SendChatCommand("/buyphonenum"); break;
			case 14: pNetGame->SendChatCommand("/vipsilv"); break;
			case 15: pNetGame->SendChatCommand("/vipgld"); break;
			case 16: pNetGame->SendChatCommand("/vippltnum"); break;
		}
	}
	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeHudElementColor(JNIEnv* pEnv, jobject thiz, jint id, jint a, jint r, jint g, jint b)
	{
		CAdjustableHudColors::SetHudColorFromRGBA((E_HUD_ELEMENT)id, r, g, b, a);
	}

	JNIEXPORT jbyteArray JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeHudElementColor(JNIEnv* pEnv, jobject thiz, jint id)
	{
		char pTemp[9];
		jbyteArray color = pEnv->NewByteArray(sizeof(pTemp));

		if (!color)
		{
			return nullptr;
		}

		pEnv->SetByteArrayRegion(color, 0, sizeof(pTemp), (const jbyte*)CAdjustableHudColors::GetHudColorString((E_HUD_ELEMENT)id).c_str());

		return color;
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeHudElementPosition(JNIEnv* pEnv, jobject thiz, jint id, jint x, jint y)
	{
		if (id == 7)
		{
			if (pSettings)
			{
				pSettings->GetWrite().fChatPosX = x;
				pSettings->GetWrite().fChatPosY = y;
				if (pChatWindow)
				{
					pChatWindow->m_bPendingReInit = true;
				}
				return;
			}
			return;
		}
		if (id == HUD_SNOW)
		{
			if (pSettings)
			{
				pSettings->GetWrite().iSnow = x;
			}
			CSnow::SetCurrentSnow(pSettings->GetReadOnly().iSnow);
			return;
		}
		CAdjustableHudPosition::SetElementPosition((E_HUD_ELEMENT)id, x, y);

		if (id >= HUD_WEAPONSPOS && id <= HUD_WEAPONSROT)
		{
			CWeaponsOutFit::OnUpdateOffsets();
		}
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeHudElementScale(JNIEnv* pEnv, jobject thiz, jint id, jint x, jint y)
	{
		CAdjustableHudScale::SetElementScale((E_HUD_ELEMENT)id, x, y);

		if (id >= HUD_WEAPONSPOS && id <= HUD_WEAPONSROT)
		{
			CWeaponsOutFit::OnUpdateOffsets();
		}
	}

	JNIEXPORT jintArray JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeHudElementScale(JNIEnv* pEnv, jobject thiz, jint id)
	{
		jintArray color = pEnv->NewIntArray(2);

		if (!color)
		{
			return nullptr;
		}
		int arr[2];
		arr[0] = CAdjustableHudScale::GetElementScale((E_HUD_ELEMENT)id).X;
		arr[1] = CAdjustableHudScale::GetElementScale((E_HUD_ELEMENT)id).Y;
		pEnv->SetIntArrayRegion(color, 0, 2, (const jint*)& arr[0]);

		return color;
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_setNativeWidgetPositionAndScale(JNIEnv* pEnv, jobject thiz, jint id, jint x, jint y, jint scale)
	{
		if (id == 0)
		{
			if (pSettings)
			{
				pSettings->GetWrite().fButtonMicrophoneX = x;
				pSettings->GetWrite().fButtonMicrophoneY = y;
				pSettings->GetWrite().fButtonMicrophoneSize = scale;
			}

			if (g_pWidgetManager)
			{
				if (g_pWidgetManager->GetSlotState(WIDGET_MICROPHONE))
				{
					g_pWidgetManager->GetWidget(WIDGET_MICROPHONE)->SetPos(x, y);
					g_pWidgetManager->GetWidget(WIDGET_MICROPHONE)->SetHeight(scale);
					g_pWidgetManager->GetWidget(WIDGET_MICROPHONE)->SetWidth(scale);
				}
			}
		}
		
		if (id == 1)
		{
			if (pSettings)
			{
				pSettings->GetWrite().fButtonEnterPassengerX = x;
				pSettings->GetWrite().fButtonEnterPassengerY = y;
				pSettings->GetWrite().fButtonEnterPassengerSize = scale;
			}
		}

		if (id == 2)
		{
			if (pSettings)
			{
				pSettings->GetWrite().fButtonCameraCycleX = x;
				pSettings->GetWrite().fButtonCameraCycleY = y;
				pSettings->GetWrite().fButtonCameraCycleSize = scale;
			}

			if (g_pWidgetManager)
			{
				if (g_pWidgetManager->GetSlotState(WIDGET_CAMERA_CYCLE))
				{
					g_pWidgetManager->GetWidget(WIDGET_CAMERA_CYCLE)->SetPos(x, y);
					g_pWidgetManager->GetWidget(WIDGET_CAMERA_CYCLE)->SetHeight(scale);
					g_pWidgetManager->GetWidget(WIDGET_CAMERA_CYCLE)->SetWidth(scale);
				}
			}
		}
	}

	JNIEXPORT jintArray JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeHudElementPosition(JNIEnv* pEnv, jobject thiz, jint id)
	{
		jintArray color = pEnv->NewIntArray(2);

		if (!color)
		{
			return nullptr;
		}
		int arr[2];

		if (id == 7 && pSettings)
		{
			arr[0] = pSettings->GetReadOnly().fChatPosX;
			arr[1] = pSettings->GetReadOnly().fChatPosY;
		}
		else if (id == HUD_SNOW && pSettings)
		{
			arr[0] = CSnow::GetCurrentSnow();
			arr[1] = CSnow::GetCurrentSnow();
		}
		else
		{
			arr[0] = CAdjustableHudPosition::GetElementPosition((E_HUD_ELEMENT)id).X;
			arr[1] = CAdjustableHudPosition::GetElementPosition((E_HUD_ELEMENT)id).Y;
		}

		pEnv->SetIntArrayRegion(color, 0, 2, (const jint*)&arr[0]);

		return color;
	}

	JNIEXPORT jintArray JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_getNativeWidgetPositionAndScale(JNIEnv* pEnv, jobject thiz, jint id)
	{
		jintArray color = pEnv->NewIntArray(3);

		if (!color)
		{
			return nullptr;
		}
		int arr[3] = { -1, -1, -1 };
		

		if (pSettings)
		{
			if (id == 0)
			{
				arr[0] = pSettings->GetWrite().fButtonMicrophoneX;
				arr[1] = pSettings->GetWrite().fButtonMicrophoneY;
				arr[2] = pSettings->GetWrite().fButtonMicrophoneSize;
			}
			if (id == 1)
			{
				arr[0] = pSettings->GetWrite().fButtonEnterPassengerX;
				arr[1] = pSettings->GetWrite().fButtonEnterPassengerY;
				arr[2] = pSettings->GetWrite().fButtonEnterPassengerSize;
			}
			if (id == 2)
			{
				arr[0] = pSettings->GetWrite().fButtonCameraCycleX;
				arr[1] = pSettings->GetWrite().fButtonCameraCycleY;
				arr[2] = pSettings->GetWrite().fButtonCameraCycleSize;
			}
		}
		

		pEnv->SetIntArrayRegion(color, 0, 3, (const jint*)& arr[0]);

		return color;
	}
	
	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_sendButton(JNIEnv* pEnv, jobject thiz, jint type, jbyteArray str, jint action)
	{
		CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
		if (pPlayerPed)
		{
			if (pNetGame && !pDialogWindow->m_bIsActive && pGame->IsToggledHUDElement(HUD_ELEMENT_BUTTONS))
			{
				switch(type) {
					case 1:
					switch(action) {
						case 1:
						{
							CTextDrawPool *pTextDrawPool = pNetGame->GetTextDrawPool();
							RakNet::BitStream bsClick;
							bsClick.Write(0xFFFF);
							pNetGame->GetRakClient()->RPC(&RPC_ClickTextDraw, &bsClick, HIGH_PRIORITY, RELIABLE_ORDERED, 0, false, UNASSIGNED_NETWORK_ID, nullptr);
							pTextDrawPool->SetSelectState(false, 0);
							break;
						}
						case 2:
						{
							if (!pScoreBoard->m_bToggle) 
							{
								pScoreBoard->Toggle();
							}
							else 
							{
								pScoreBoard->Toggle();
							}
							break;
						}
						case 3:
						{
							if(pPlayerPed->IsInVehicle()) LocalPlayerKeys.bKeys[ePadKeys::KEY_FIRE] = true;
							else LocalPlayerKeys.bKeys[ePadKeys::KEY_WALK] = true;
							break;
						}
						case 4:
						{
							LocalPlayerKeys.bKeys[ePadKeys::KEY_SECONDARY_ATTACK] = true;
							break;
						}
						case 5:
						{
							LocalPlayerKeys.bKeys[ePadKeys::KEY_CTRL_BACK] = true;

							CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
							CLocalPlayer *pLocalPlayer = pPlayerPool->GetLocalPlayer();
							int iWeapon = pPlayerPed->GetCurrentWeapon();
							if(iWeapon != WEAPON_FIST) 
							{
								if(!pLocalPlayer->m_bPassengerDriveByMode) 
								{
									if(pPlayerPed->StartPassengerDriveByMode(true)) 
										pLocalPlayer->m_bPassengerDriveByMode = true;
								}
								else
								{
									pGame->FindPlayerPed()->TogglePlayerControllable(false);
									pGame->FindPlayerPed()->TogglePlayerControllable(true);
									pLocalPlayer->m_bPassengerDriveByMode = false;
								}
							}
							break;
						}
						case 6:
						{
							LocalPlayerKeys.bKeys[ePadKeys::KEY_YES] = true;
							break;
						}
						case 7:
						{
							LocalPlayerKeys.bKeys[ePadKeys::KEY_NO] = true;
							break;
						}
					}
				}
			}
		}
	}

	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_sendButtonQuick(JNIEnv* pEnv, jobject thiz, jint type, jbyteArray str, jint action)
	{
		switch(type) {
			case 1:
			switch(action) {
				case 1:
				{
					pNetGame->SendChatCommand("/openinventory");
					break;
				}
				case 2:
				{
					pNetGame->SendChatCommand("/menu");
					break;
				}
				case 3:
				{
					pNetGame->SendChatCommand("/gps");
					break;
				}
				case 4:
				{
					pNetGame->SendChatCommand("/dichvu taxi");
					break;
				}
				case 5:
				{
					//g_pJavaWrapper->ShowClientSettings();
					break;
				}
			}
		}
	}
	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_buycar(JNIEnv* pEnv, jobject thiz, jint id, jint cost, jint action)
	{
		switch(action) {
				case 1: 
					pNetGame->SendDonateCar(id, cost); break;
				}
	}


	JNIEXPORT void JNICALL Java_com_nvidia_devtech_NvEventQueueActivity_sendCommand(JNIEnv* pEnv, jobject thiz, jbyteArray str)
	{
		jboolean isCopy = true;

		jbyte* pMsg = pEnv->GetByteArrayElements(str, &isCopy);
		jsize length = pEnv->GetArrayLength(str);

		std::string szStr((char*)pMsg, length);

		if(pNetGame) {
			pNetGame->SendChatCommand((char*)szStr.c_str());
		}

		pEnv->ReleaseByteArrayElements(str, pMsg, JNI_ABORT);
	}
}

void CJavaWrapper::ShowHud()
{
    JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}
    env->CallVoidMethod(this->activity, this->s_showHud);
}

void CJavaWrapper::HideHud()
{
    JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}
    env->CallVoidMethod(this->activity, this->s_hideHud);
}

void CJavaWrapper::SetPauseState(bool a1)
{
    JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}
    env->CallVoidMethod(this->activity, this->s_setPauseState, a1);
}

void CJavaWrapper::UpdateHudInfo(int health, int armour, int weaponid, int ammo, int ammoinclip, int money, int wanted)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(this->activity, this->s_updateHudInfo, health, armour, weaponid, ammo, ammoinclip, money, wanted);
}

void CJavaWrapper::ShowTabWindow()
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}
	env->CallVoidMethod(this->activity, this->s_showTabWindow);
}

void CJavaWrapper::SetTabStat(int id, char* name, int score, int ping) {

	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	jclass strClass = env->FindClass("java/lang/String");
    jmethodID ctorID = env->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    jstring encoding = env->NewStringUTF("UTF-8");

    jbyteArray bytes = env->NewByteArray(strlen(name));
    env->SetByteArrayRegion(bytes, 0, strlen(name), (jbyte*)name);
    jstring jname = (jstring) env->NewObject(strClass, ctorID, bytes, encoding);

	env->CallVoidMethod(this->activity, this->s_setTabStat, id, jname, score, ping);
}

void CJavaWrapper::ShowSpeed()
{
    JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}
    env->CallVoidMethod(this->activity, this->s_showSpeed);
}

void CJavaWrapper::HideSpeed()
{
    JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}
    env->CallVoidMethod(this->activity, this->s_hideSpeed);
}

void CJavaWrapper::UpdateSpeedInfo(int speed, int fuel, int hp, int mileage, int engine, int light, int belt, int lock)
{
	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	env->CallVoidMethod(this->activity, this->s_updateSpeedInfo, speed, fuel, hp, mileage, engine, light, belt, lock);
}

void CJavaWrapper::ShowWelcome(bool a) {

    JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}
    env->CallVoidMethod(this->activity, this->s_showWelcome, a);
}

void CJavaWrapper::ShowPhone()
{
    JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}
    env->CallVoidMethod(this->activity, this->s_showphone);
}

void CJavaWrapper::HidePhone()
{
    JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}
    env->CallVoidMethod(this->activity, this->s_hidephone);
}

void CJavaWrapper::ShowNotification(int delay, const char* name, const char* msg, const char* url) {

	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	jclass strClass = env->FindClass("java/lang/String");
    jmethodID ctorID = env->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    jstring encoding = env->NewStringUTF("UTF-8");

    jbyteArray bytes = env->NewByteArray(strlen(name));
    env->SetByteArrayRegion(bytes, 0, strlen(name), (jbyte*)name);
    jstring jname = (jstring) env->NewObject(strClass, ctorID, bytes, encoding);
	////
	jclass strClass1 = env->FindClass("java/lang/String");
    jmethodID ctorID1 = env->GetMethodID(strClass1, "<init>", "([BLjava/lang/String;)V");
    jstring encoding1 = env->NewStringUTF("UTF-8");

    jbyteArray bytes1 = env->NewByteArray(strlen(msg));
    env->SetByteArrayRegion(bytes1, 0, strlen(msg), (jbyte*)msg);
    jstring jmsg = (jstring) env->NewObject(strClass1, ctorID1, bytes1, encoding1);
	////
	jclass strClass2 = env->FindClass("java/lang/String");
    jmethodID ctorID2 = env->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    jstring encoding2 = env->NewStringUTF("UTF-8");

    jbyteArray bytes2 = env->NewByteArray(strlen(url));
    env->SetByteArrayRegion(bytes2, 0, strlen(url), (jbyte*)url);
    jstring jurl = (jstring) env->NewObject(strClass2, ctorID2, bytes2, encoding2);
	env->CallVoidMethod(this->activity, this->s_notification, delay, jname, jmsg, jurl);
}

void CJavaWrapper::PostTwitter(const char* msg, int duration, const char* url, const char* playername) {

	JNIEnv* env = GetEnv();

	if (!env)
	{
		Log("No env");
		return;
	}

	jclass strClass = env->FindClass("java/lang/String");
    jmethodID ctorID = env->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    jstring encoding = env->NewStringUTF("UTF-8");

    jbyteArray bytes = env->NewByteArray(strlen(msg));
    env->SetByteArrayRegion(bytes, 0, strlen(msg), (jbyte*)msg);
    jstring jmsg = (jstring) env->NewObject(strClass, ctorID, bytes, encoding);
	////
	jclass strClass1 = env->FindClass("java/lang/String");
    jmethodID ctorID1 = env->GetMethodID(strClass1, "<init>", "([BLjava/lang/String;)V");
    jstring encoding1 = env->NewStringUTF("UTF-8");

    jbyteArray bytes1 = env->NewByteArray(strlen(url));
    env->SetByteArrayRegion(bytes1, 0, strlen(url), (jbyte*)url);
    jstring jurl = (jstring) env->NewObject(strClass1, ctorID1, bytes1, encoding1);
	////
	jclass strClass2 = env->FindClass("java/lang/String");
    jmethodID ctorID2 = env->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    jstring encoding2 = env->NewStringUTF("UTF-8");

    jbyteArray bytes2 = env->NewByteArray(strlen(playername));
    env->SetByteArrayRegion(bytes2, 0, strlen(playername), (jbyte*)playername);
    jstring jplayername = (jstring) env->NewObject(strClass2, ctorID2, bytes2, encoding2);
	env->CallVoidMethod(this->activity, this->s_posttwitter, jmsg, duration, jurl, jplayername);
}

CJavaWrapper::CJavaWrapper(JNIEnv* env, jobject activity)
{
	this->activity = env->NewGlobalRef(activity);

	jclass nvEventClass = env->GetObjectClass(activity);
	if (!nvEventClass)
	{
		Log(OBFUSCATE("nvEventClass null"));
		return;
	}

	s_GetClipboardText = env->GetMethodID(nvEventClass, OBFUSCATE("getClipboardText"), OBFUSCATE("()[B"));

	s_ShowInputLayout = env->GetMethodID(nvEventClass, OBFUSCATE("showInputLayout"), OBFUSCATE("()V"));
	s_HideInputLayout = env->GetMethodID(nvEventClass, OBFUSCATE("hideInputLayout"), OBFUSCATE("()V"));

	s_ShowClientSettings = env->GetMethodID(nvEventClass, OBFUSCATE("showClientSettings"), OBFUSCATE("()V"));
	s_SetUseFullScreen = env->GetMethodID(nvEventClass, OBFUSCATE("setUseFullscreen"), OBFUSCATE("(I)V"));
	
	s_MakeDialog = env->GetMethodID(nvEventClass, OBFUSCATE("showDialog"),OBFUSCATE("(IILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V"));
	    
	s_updateHudInfo = env->GetMethodID(nvEventClass, OBFUSCATE("updateHudInfo"), OBFUSCATE("(IIIIIII)V"));
    s_showHud = env->GetMethodID(nvEventClass, OBFUSCATE("showHud"), OBFUSCATE("()V"));
    s_hideHud = env->GetMethodID(nvEventClass, OBFUSCATE("hideHud"), OBFUSCATE("()V"));
	
	s_updateSpeedInfo = env->GetMethodID(nvEventClass, OBFUSCATE("updateSpeedInfo"), OBFUSCATE("(IIIIIIII)V"));
    s_showSpeed = env->GetMethodID(nvEventClass, OBFUSCATE("showSpeed"), OBFUSCATE("()V"));
    s_hideSpeed = env->GetMethodID(nvEventClass, OBFUSCATE("hideSpeed"), OBFUSCATE("()V"));
	
	
	s_showTabWindow = env->GetMethodID(nvEventClass, OBFUSCATE("showTabWindow"), OBFUSCATE("()V"));	
	s_setTabStat = env->GetMethodID(nvEventClass, OBFUSCATE("setTabStat"), OBFUSCATE("(ILjava/lang/String;II)V"));

	s_showWelcome = env->GetMethodID(nvEventClass, OBFUSCATE("showWelcome"), OBFUSCATE("(Z)V"));	
	
	s_setPauseState = env->GetMethodID(nvEventClass, OBFUSCATE("setPauseState"), OBFUSCATE("(Z)V"));
	s_showphone = env->GetMethodID(nvEventClass, OBFUSCATE("showPhone"), OBFUSCATE("()V"));
    s_hidephone = env->GetMethodID(nvEventClass, OBFUSCATE("hidePhone"), OBFUSCATE("()V"));
	s_posttwitter = env->GetMethodID(nvEventClass, OBFUSCATE("Posttwitter"), OBFUSCATE("(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;)V"));
	s_notification = env->GetMethodID(nvEventClass, OBFUSCATE("showNotification"), OBFUSCATE("(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V"));

	s_showDonate = env->GetMethodID(nvEventClass, "showDonate", "(II)V");
	s_updateDonate = env->GetMethodID(nvEventClass, "updateDonate", "(II)V");
	s_show_sc = env->GetMethodID(nvEventClass, "show_sc", "(II)V");

	env->DeleteLocalRef(nvEventClass);
}

CJavaWrapper::~CJavaWrapper()
{
	JNIEnv* pEnv = GetEnv();
	if (pEnv)
	{
		pEnv->DeleteGlobalRef(this->activity);
	}
}

CJavaWrapper* g_pJavaWrapper = nullptr;
