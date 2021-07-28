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
	bool isAimbotOn = false;
	bool guiVisible = false;
	bool isESPTextOn = false;
	int espMode = 0;
public:
	int key_GUI = VK_INSERT;
public:
	void UpdateKey();
public:
	struct {
		bool friendlyFire = false;
		float smoothness = 10.f;
	} Aimbot;
};

