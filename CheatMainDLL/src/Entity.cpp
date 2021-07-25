#include "stdafx.h"
#include "Entity.h"
#include "csgo.h"

// Entity Classes
Entity::Entity(void* baseparam) {
	if (baseparam == nullptr) {
		throw EntityException(__LINE__, __FILE__, "The Entity constructor received nullptr for base address.", EntityException::errCodeEnum::EntityNotAvaliable);
	}
	base = (uintptr_t*)baseparam;
	init();
}

// Offsets(Pointers) Initalizer
void Entity::init() {
	// Update Offsets
	m_iHealth = (int*)((uintptr_t)base + netvars::m_iHealth);
	m_iTeamNum = (int*)((uintptr_t)base + netvars::m_iTeamNum);
	m_fFlags = (DWORD*)((uintptr_t)base + netvars::m_fFlags);
	m_iShotsFired = (int*)((uintptr_t)base + netvars::m_iShotsFired);
	m_vecVelocity = (Vec3*)((uintptr_t)base + netvars::m_vecVelocity);
	m_dwBoneMatrix = (DWORD*)((uintptr_t)base + netvars::m_dwBoneMatrix);
	m_bDormant = (bool*)((uintptr_t)base + signatures::m_bDormant);
	m_vecOrigin = (Vec3*)((uintptr_t)base + netvars::m_vecOrigin);
	m_ArmorValue = (int*)((uintptr_t)base + netvars::m_ArmorValue);
	m_angEyeAngles = (Vec2*)((uintptr_t)base + netvars::m_angEyeAnglesX);
	m_vecViewOffset = (Vec3*)((uintptr_t)base + netvars::m_vecViewOffset);
	// Update Weapon ID
	curWeapon.crosshairEntityId = (int*)((uintptr_t)base + netvars::m_iCrosshairId);
}

Entity::EntityException::EntityException(int line, const char* file, const char* err, errCodeEnum errcode) : errCode(errcode), BHException(line, file, err) {}
const char* Entity::EntityException::GetType() const noexcept { return "EntityException"; }
std::string Entity::EntityException::GetErrorString() const noexcept {
	switch (errCode) {
	case EntityNotAvaliable:
		return "Entity is not avaliable.";
	case EntityNotValidPlayer:
		return "The entity you requested is not a valid player.";
	case EntityOutOfRange:
		return "Entity List out of range.";
	}
	return "Unknown Error.";
}

bool Entity::isOnGround() const { return (*m_fFlags) & (1 << 0); }
bool Entity::isCroughing() const { return (*m_fFlags) & (1 << 1); }
bool Entity::isJumpingOutOfWater() const { return (*m_fFlags) & (1 << 2); }
bool Entity::isOnTrain() const { return (*m_fFlags) & (1 << 3); }
bool Entity::isStandingOnRain() const { return (*m_fFlags) & (1 << 4); }
bool Entity::isFrozen() const { return (*m_fFlags) & (1 << 5); }
bool Entity::isAtControls() const { return (*m_fFlags) & (1 << 6); }
bool Entity::isClient() const { return (*m_fFlags) & (1 << 7); }
bool Entity::isFakeClient() const { return (*m_fFlags) & (1 << 8); }
bool Entity::isInWater() const { return (*m_fFlags) & (1 << 9); }
bool Entity::isMoving() const { return m_vecVelocity->x + m_vecVelocity->y + m_vecVelocity->z; }
bool Entity::isDormant() const { return *m_bDormant; }
bool Entity::isValidPlayer() const { return GetHealth() > 0 && !isDormant(); }
unsigned int Entity::GetCurrentWeapon() const { return curWeapon.id; }
unsigned int Entity::GetCurrentCrosshair() const { return *curWeapon.crosshairEntityId; }
uintptr_t Entity::GetBaseAddr() const { return (uintptr_t)base; }
int Entity::GetShotsFired() const { return *m_iShotsFired; }
int Entity::GetHealth() const { return *m_iHealth; }
int Entity::GetTeamNum() const { return *m_iTeamNum; }
Vec3 Entity::GetPos() const { return *(Vec3*)m_vecOrigin; }
Vec2 Entity::GetEyeAngle() const { return *(Vec2*)m_angEyeAngles; }
Vec3 Entity::GetVelocity() const { return *(Vec3*)m_vecVelocity; }
Vec3 Entity::GetBonePos(int index) const {
	uintptr_t bonePtr = *m_dwBoneMatrix;
	Vec3 bonePos;
	bonePos.x = *(float*)(bonePtr + 0x30 * index + 0x0C);
	bonePos.y = *(float*)(bonePtr + 0x30 * index + 0x1C);
	bonePos.z = *(float*)(bonePtr + 0x30 * index + 0x2C);
	return bonePos;
}
Vec3 Entity::GetEyePos() const {
	return *m_vecOrigin + *m_vecViewOffset;
}