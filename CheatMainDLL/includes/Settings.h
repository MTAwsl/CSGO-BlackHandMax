#pragma once
#define REGISTER_SWITCH_KEY(key, boolval) if (GetAsyncKeyState(key) & 1){ boolval = !boolval;}
#define REGISTER_MODE_KEY(key, intval, modeindex) if (GetAsyncKeyState(key) & 1){intval++; intval %= modeindex;}

class Settings
{
public:
	bool isRCSOn = false;
	bool isTriggerbotOn = false;
	bool isBhopOn = false;
	bool isSnaplineOn = false;
	unsigned int espMode = 0;
public:
	int key_RCS = VK_INSERT;
	int key_Triggerbot = VK_DELETE;
	int key_Bhop = VK_PAUSE;
	int key_ESP = VK_HOME;
	int key_Snapline = VK_PRIOR;
public:
	void UpdateKey();
};

