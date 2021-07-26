#include "stdafx.h"
#include "csgo.h"
#include "Entity.h"
#include "Module.h"

constexpr int BONE_HEAD = 8;

// Module Classes
Module::Module(LPCWSTR name) {
	base = (DWORD)(GetModuleHandle(name));
	init(); // For initalize pointers
}

void Module::init() {}

DWORD Module::GetModuleBase(){ return base; }

ClientModule::ClientModule() : Module(L"client.dll"), localPlayer{ Player(this) } {}
EngineModule::EngineModule() : Module(L"engine.dll") {}

// Player Functions
ClientModule::Player::Player(ClientModule* pClient) 
try : pClientMod(pClient), Entity((uintptr_t*)*(uintptr_t*)(pClientMod->GetModuleBase() + signatures::dwLocalPlayer)) {}
catch (Entity::EntityException& e) {
	if (e.errCode != e.EntityNotAvaliable)
		throw e;
}

void ClientModule::Player::Jump() {
	*dwForceJump = 0b0000'0101; // Setting the bitMask of the game
	opMask.set(MASK_JUMP); // Setting bitMask to let the handler revert this
}

void ClientModule::Player::Shoot() {
	*dwForceAttack = 0b0000'0101; // Setting the bitMask of the game
	opMask.set(MASK_ATTACK); // Setting bitMask to let the handler revert this
}
void ClientModule::Player::Rebase(void* ptr)
{
	base = (uintptr_t*)ptr;
	this->init();
}
ViewAngle ClientModule::Player::GetAimPunchAngle() const { return *m_aimPunchAngle; }
Entity ClientModule::GetEntity(int index) const {
	void* ptr = pEntityList[index].entptr;
	if (!ptr)
		throw Entity::EntityException(__LINE__, __FILE__, "The entity you requested is not avaliable.", Entity::EntityException::EntityNotAvaliable);
	return Entity((pEntityList[index]).entptr);
}
bool ClientModule::GetViewMatrix(float* VMatrix) const {
	if (!dwViewMatrix or !VMatrix)
		return false;
	memcpy(VMatrix, dwViewMatrix, sizeof(float) * 16);
	return true;
}
ViewAngle EngineModule::GetViewAngle() const {
	return *dwClientState_ViewAngles;
}
void EngineModule::SetViewAngle(ViewAngle angle) {
	*dwClientState_ViewAngles = angle;
}

void ClientModule::Player::init() {
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
	m_aimPunchAngle = (ViewAngle*)((uintptr_t)base + netvars::m_aimPunchAngle);
	// Update Weapon ID
	curWeapon.crosshairEntityId = (int*)(*base + netvars::m_iCrosshairId);
}

void ClientModule::Reset() {
	// Reset BitMasks
	if (opMask.test(MASK_JUMP)) {
		*dwForceJump = 0b0000'0100;
		opMask.reset(MASK_JUMP);
	}
	if (opMask.test(MASK_ATTACK)) {
		*dwForceAttack = 0b0000'0100;
		opMask.reset(MASK_ATTACK);
	}
}

void ClientModule::init() {
	try {
		localPlayer.Rebase((uintptr_t*)*(uintptr_t*)(GetModuleBase() + signatures::dwLocalPlayer));
	}
	catch (Entity::EntityException& e) {
		if (e.errCode != e.EntityNotAvaliable)
			throw e;
	}
	// Update Offsets
	dwForceJump = (DWORD*)(GetModuleBase() + signatures::dwForceJump);
	dwForceAttack = (DWORD*)(GetModuleBase() + signatures::dwForceAttack);
	dwViewMatrix = (DWORD*)(GetModuleBase() + signatures::dwViewMatrix);
	pEntityList = (clientInfo*)(GetModuleBase() + signatures::dwEntityList);
}

void EngineModule::init() {
	// Update Offsets
	dwClientState = (DWORD*)(GetModuleBase() + signatures::dwClientState);
	dwClientState_ViewAngles = (ViewAngle*)(*dwClientState + signatures::dwClientState_ViewAngles);
}


// Static Variables
DWORD* ClientModule::dwForceAttack = nullptr;
DWORD* ClientModule::dwForceJump = nullptr;
DWORD* ClientModule::dwViewMatrix = nullptr;
std::bitset<8> ClientModule::opMask = { 0 };
fEndScene Graphics::originalEndScene = nullptr;
ID3DXLine* Graphics::LineL = nullptr;
LPDIRECT3DDEVICE9 Graphics::pDevice = nullptr;
HWND Graphics::hWnd = nullptr;