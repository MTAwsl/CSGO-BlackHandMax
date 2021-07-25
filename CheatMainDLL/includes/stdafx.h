#pragma once
#include <iostream>
#include <Windows.h>
#include <bitset>
#include <d3d9.h>
#include <d3dx9.h>
#include <cstddef>
#include <mutex>
#include <exception>
#include <sstream>

#define _USE_MATH_DEFINES
#include <math.h>

#include "exception.h"
#include "Game.h"
#include "Aimbot.h"
#include "Hook.h"
#include "Graphics.h"
#include "Entity.h"
#pragma comment (lib,"d3d9.lib")
#pragma comment (lib,"d3dx9.lib")

#define TORAD(x) x * (M_PI / 180.f)
#define TODEG(x) x * (180.f / M_PI)