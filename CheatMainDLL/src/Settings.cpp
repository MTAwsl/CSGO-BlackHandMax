#include "stdafx.h"
#include "Settings.h"

void Settings::UpdateKey() {
	REGISTER_MODE_KEY(key_ESP, espMode, 3);
	REGISTER_SWITCH_KEY(key_Bhop, isBhopOn);
	REGISTER_SWITCH_KEY(key_RCS, isRCSOn);
	REGISTER_SWITCH_KEY(key_Triggerbot, isTriggerbotOn);
	REGISTER_SWITCH_KEY(key_Snapline, isSnaplineOn);
	REGISTER_SWITCH_KEY(key_Aimbot, isAimbotOn);
}