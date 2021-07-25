#pragma once
#include "stdafx.h"

class Settings;

class Aimbot {
public:
	Aimbot(Cheat* pHax);
	void run();
private:
	Cheat* pHax;
};