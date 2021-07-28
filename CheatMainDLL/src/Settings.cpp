#include "stdafx.h"
#include "Settings.h"

void Settings::UpdateKey() {
	REGISTER_SWITCH_KEY(key_GUI, guiVisible);
}