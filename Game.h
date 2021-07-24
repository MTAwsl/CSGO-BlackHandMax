#pragma once
#include "types.h"
#include "Graphics.h"
#include <stdint.h>
#include <bitset>

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

class Entity {
public:
	Entity(void* baseparam);
	virtual void init();
public:
	// fFlags states
	bool isOnGround() const;
	bool isCroughing() const;
	bool isJumpingOutOfWater() const;
	bool isOnTrain() const;
	bool isStandingOnRain() const;
	bool isFrozen() const;
	bool isAtControls() const;
	bool isClient() const;
	bool isFakeClient() const;
	bool isInWater() const;
	// isDormant
	bool isDormant() const;
	// Custom states
	bool isMoving() const;
	bool isValidPlayer() const;
public:
	uintptr_t GetBaseAddr() const;
	unsigned int GetCurrentWeapon() const;
	unsigned int GetCurrentCrosshair() const;
	int GetShotsFired() const;
	int GetHealth() const;
	int GetTeamNum() const;
	Vec3 GetPos() const;
	Vec3 GetBonePos(int index) const;
	Vec2 GetEyeAngle() const;
protected:
	uintptr_t* base = nullptr;
	int* m_iHealth = nullptr;
	int* m_iTeamNum = nullptr;
	DWORD* m_fFlags = nullptr;
	int* m_iShotsFired = nullptr;
	Vec3* m_vecVelocity = nullptr;
	Vec3* m_vecOrigin = nullptr;
	Vec2* m_angEyeAngles = nullptr;
	int* m_ArmorValue = nullptr;
	DWORD* m_dwBoneMatrix = nullptr;
	bool* m_bDormant = nullptr;
	class Weapon {
	public:
		unsigned int id = 0;
		int* crosshairEntityId = nullptr;
	} curWeapon;
};

class ClientModule : public Module {
public:
	class Player : public Entity {
	public:
		Player(void* baseaddr);
		void init(); // Initalize pointers
	public:
		// Player Operations
		void Jump();
		void Shoot();
	public:
		ViewAngle GetAimPunchAngle() const;
	private:
		ViewAngle* m_aimPunchAngle = nullptr;
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

class Game
{
public:
	bool isRCSOn = false;
	bool isTriggerbotOn = false;
	bool isBhopOn = false;
	bool isESPOn = false;
public:
	Game();
	void HandleTriggerbot();
	void HandleRCS();
	void HandleBunnyhop();
	void DrawESP();
public:
	ClientModule modClient;
	EngineModule modEngine;
	Graphics gpu;
};