#pragma once
#include <iostream>
#include <Windows.h>
#include <bitset>
#include <d3d9.h>
#include <d3dx9.h>
#include <cstddef>
#include <mutex>

#define _USE_MATH_DEFINES
#include <math.h>

#include "Game.h"
#include "Hook.h"
#include "Graphics.h"
#pragma comment (lib,"d3d9.lib")
#pragma comment (lib,"d3dx9.lib")

#define TORAD(x) x * (M_PI / 180.f)