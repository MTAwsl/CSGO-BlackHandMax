#pragma once
#include "stdafx.h"

class Cheat
{
public:
	Cheat();
	void HandleTriggerbot();
	void HandleRCS();
	void HandleBunnyhop();
	void DrawESP();
public:
	Aimbot Aimbot;
public:
	ClientModule modClient;
	EngineModule modEngine;
	Graphics gpu;
	Settings settings;
};
