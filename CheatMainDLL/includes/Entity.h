#pragma once
#include "stdafx.h"
#include <Windows.h>
class Entity {
public:
	Entity(void* baseparam, bool noWarnings = false);
	virtual void init();
public:
	// fFlags states
	bool isOnGround() const noexcept;
	bool isCroughing() const noexcept;
	bool isJumpingOutOfWater() const noexcept;
	bool isOnTrain() const noexcept;
	bool isStandingOnRain() const noexcept;
	bool isFrozen() const noexcept;
	bool isAtControls() const noexcept;
	bool isClient() const noexcept;
	bool isFakeClient() const noexcept;
	bool isInWater() const noexcept;
	// bool States
	bool isHelmet() const noexcept;
	bool isDormant() const noexcept;
	// Custom states
	bool isMoving() const noexcept;
	bool isValidPlayer() const noexcept;
public:
	uintptr_t GetBaseAddr() const noexcept;
	unsigned int GetCurrentWeapon() const noexcept;
	unsigned int GetCurrentCrosshair() const noexcept;
	int GetShotsFired() const noexcept;
	int GetHealth() const noexcept;
	int GetTeamNum() const noexcept;
	int GetArmorValue() const noexcept;
	Vec3 GetPos() const noexcept;
	Vec3 GetBonePos(int index) const noexcept;
	Vec2 GetEyeAngle() const noexcept;
	Vec3 GetVelocity() const noexcept;
	Vec3 GetEyePos() const noexcept;
protected:
	uintptr_t* base = nullptr;
	int* m_iHealth = nullptr;
	int* m_iTeamNum = nullptr;
	int* m_iShotsFired = nullptr;
	int* m_ArmorValue = nullptr;
	DWORD* m_fFlags = nullptr;
	DWORD* m_dwBoneMatrix = nullptr;
	Vec3* m_vecVelocity = nullptr;
	Vec3* m_vecViewOffset = nullptr;
	Vec3* m_vecOrigin = nullptr;
	Vec2* m_angEyeAngles = nullptr;
	bool* m_bDormant = nullptr;
	bool* m_bHasHelmet = nullptr;
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