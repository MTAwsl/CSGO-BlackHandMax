#pragma once
#include "types.h"
#include "Graphics.h"
#include "Entity.h"
#include <stdint.h>
#include <bitset>
#include "Settings.h"
#include "Aimbot.h"

class clientInfo
{
public:
	class Entity* entptr; //0x0000
	int32_t unkVal; //0x0004
	class clientInfo* pPrev;
	class clientInfo* pNext;
};

class Module {
public:
	Module(LPCWSTR name);
	DWORD GetModuleBase();
	virtual void init();
private:
	DWORD base;
};

class ClientModule : public Module {
public:
	class Player : public Entity {
	public:
		Player(ClientModule* pClient);
		void init(); // Initalize pointers
	public:
		// Player Operations
		void Jump();
		void Shoot();
		void Rebase(void* ptr);
	public:
		ViewAngle GetAimPunchAngle() const;
	private:
		ViewAngle* m_aimPunchAngle = nullptr;
		ClientModule* pClientMod;
	};
public:
	ClientModule();
	Entity GetEntity(int index) const;
	bool GetViewMatrix(float* VMatrix) const;
	void init(); // Initalize pointers
	void Reset(); // Reset Bitmasks
public:
	Player localPlayer; // Main Player Obj
private:
	clientInfo* pEntityList = nullptr;
private:
	enum opMaskEnum {
		MASK_JUMP = 0,
		MASK_ATTACK = 1
	};
	static DWORD* dwForceJump;
	static DWORD* dwForceAttack;
	static DWORD* dwViewMatrix;
	static std::bitset<8> opMask;// 0 - Jump, 1 - Attack
private:
	friend class EngineModule;
};

class EngineModule : public Module {
public:
	EngineModule();
	void init();
	ViewAngle GetViewAngle() const;
	void SetViewAngle(ViewAngle angle);
private:
	DWORD* dwClientState = nullptr;
	ViewAngle* dwClientState_ViewAngles = nullptr;
private:
	friend class ClientModule;
};