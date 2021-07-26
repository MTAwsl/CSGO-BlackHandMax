#pragma once
#include "stdafx.h"
#include <Windows.h>
class Entity {
public:
	Entity(void* baseparam, bool noWarnings = false);
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
	Vec3 GetVelocity() const;
	Vec3 GetEyePos() const;
protected:
	uintptr_t* base = nullptr;
	int* m_iHealth = nullptr;
	int* m_iTeamNum = nullptr;
	DWORD* m_fFlags = nullptr;
	int* m_iShotsFired = nullptr;
	Vec3* m_vecVelocity = nullptr;
	Vec3* m_vecViewOffset = nullptr;
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
public:
	class EntityException : public BHException {
	public:
		enum errCodeEnum {
			EntityNotAvaliable,
			EntityNotValidPlayer,
			EntityOutOfRange
		};
	public:
		EntityException(int line, const char* file, const char* err, errCodeEnum errcode);
		const char* GetType() const noexcept override;
		std::string GetErrorString() const noexcept override;
		errCodeEnum errCode;
	};
};