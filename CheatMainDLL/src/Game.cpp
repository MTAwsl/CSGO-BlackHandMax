#include "stdafx.h"
#include "csgo.h"

constexpr int BONE_HEAD = 8;

// Module Classes
Module::Module(LPCWSTR name) {
	base = (DWORD)(GetModuleHandle(name));
	init(); // For initalize pointers
}

void Module::init() {}

DWORD Module::GetModuleBase(){ return base; }

ClientModule::ClientModule() : Module(L"client.dll"), localPlayer{Player((uintptr_t*)(GetModuleBase() + signatures::dwLocalPlayer))} {}
EngineModule::EngineModule() : Module(L"engine.dll") {}

// Entity Classes
Entity::Entity(void* baseparam) {
	if (baseparam == nullptr) {
		throw EntityException(__LINE__, __FILE__, "The Entity constructor received nullptr for base address.", EntityException::errCodeEnum::EntityNotAvaliable);
	}
	base = (uintptr_t*)baseparam;
	init();
}

Entity::EntityException::EntityException(int line, const char* file, const char* err, errCodeEnum errcode) : errCode(errcode), BHException(line, file, err){}
const char* Entity::EntityException::GetType() const noexcept{ return "EntityException"; }
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
bool Entity::isMoving() const { return m_vecVelocity->x + m_vecVelocity->y + m_vecVelocity->z;}
bool Entity::isDormant() const { return *m_bDormant; }
bool Entity::isValidPlayer() const { return GetHealth() > 0 && !isDormant(); }
unsigned int Entity::GetCurrentWeapon() const { return curWeapon.id; }
unsigned int Entity::GetCurrentCrosshair() const { return *curWeapon.crosshairEntityId; }
uintptr_t Entity::GetBaseAddr() const { return (uintptr_t)base; }
int Entity::GetShotsFired() const { return *m_iShotsFired; }
int Entity::GetHealth() const { return *m_iHealth; }
int Entity::GetTeamNum() const { return *m_iTeamNum; }
Vec3 Entity::GetPos() const { return *(Vec3*)m_vecOrigin; }
Vec2 Entity::GetEyeAngle() const  {return *(Vec2*)m_angEyeAngles; }
Vec3 Entity::GetVelocity() const{ return *(Vec3*)m_vecVelocity; }
Vec3 Entity::GetBonePos(int index) const {
	uintptr_t bonePtr = *m_dwBoneMatrix;
	Vec3 bonePos;
	bonePos.x = *(float*)(bonePtr + 0x30 * index + 0x0C);
	bonePos.y = *(float*)(bonePtr + 0x30 * index + 0x1C);
	bonePos.z = *(float*)(bonePtr + 0x30 * index + 0x2C);
	return bonePos;
}

// Player Functions
ClientModule::Player::Player(void* baseaddr) : Entity(baseaddr) {}

void ClientModule::Player::Jump() {
	*dwForceJump = 0b0000'0101; // Setting the bitMask of the game
	opMask.set(MASK_JUMP); // Setting bitMask to let the handler revert this
}

void ClientModule::Player::Shoot() {
	*dwForceAttack = 0b0000'0101; // Setting the bitMask of the game
	opMask.set(MASK_ATTACK); // Setting bitMask to let the handler revert this
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

// Hack functions
Cheat::Cheat() : gpu{ Graphics::Graphics(this) } {}

void Cheat::HandleTriggerbot() {
	static unsigned int lastTick = 0;
	if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) { // Manual Override
		unsigned int crosshair = modClient.localPlayer.GetCurrentCrosshair(); // To prevent the game thread updating its data
		if (GetTickCount() - lastTick >= 150 && crosshair != 0 && crosshair < 64) {
			Entity target(modClient.GetEntity(crosshair - 1));
			if (target.GetHealth() > 0) {
				modClient.localPlayer.Shoot();
			}
			lastTick = GetTickCount();
		}
	}
}

void Cheat::HandleRCS() {
	static ViewAngle oldPunch = { 0, 0, 0 };
	ViewAngle punchAngle = modClient.localPlayer.GetAimPunchAngle() * 2;
	if (modClient.localPlayer.GetShotsFired() > 1) {
		ViewAngle newAngle = modEngine.GetViewAngle() + oldPunch - punchAngle;
		newAngle.Normalize();
		modEngine.SetViewAngle(newAngle);
	}
	oldPunch = punchAngle;
}

void Cheat::HandleBunnyhop() {
	if (modClient.localPlayer.isMoving() && modClient.localPlayer.isOnGround() && GetAsyncKeyState(VK_SPACE) & 0x8000)
		modClient.localPlayer.Jump();
}

void Cheat::DrawESP() {
	for (int i = 1; i < 32; i++) {
		float VMatrix[16];
		if (!modClient.GetViewMatrix(VMatrix)) return; // initialize VMatrix
		try {
			Entity ent = modClient.GetEntity(i); // Get Entity
			if (!ent.isValidPlayer() || ent.GetBaseAddr() == modClient.GetEntity(0).GetBaseAddr() || ent.GetBaseAddr() == modClient.localPlayer.GetBaseAddr()) // Entity 0 is localEnt (CWorld or localplayer I dont know)
				continue;
			Vec3 oriHeadPos;
			Vec3 headPos = oriHeadPos =ent.GetBonePos(BONE_HEAD);
			Vec3 playerPos = ent.GetPos();
			Vec2 eyeAngle = ent.GetEyeAngle();
			Vec2Int wndSize = gpu.GetWindowSize();
			Vec2Int headPos2D;
			Vec2Int playerPos2D;
			D3DCOLOR color = ent.GetTeamNum() != modClient.localPlayer.GetTeamNum() ? D3DCOLOR_ARGB(255, 255, 42, 50) : D3DCOLOR_ARGB(255, 42, 255, 50);

			headPos.z += 8;
			if (!gpu.WorldToScreen(headPos, headPos2D, VMatrix)) continue;
			if (!gpu.WorldToScreen(playerPos, playerPos2D, VMatrix)) continue;

			// Draw ESP SnapLine
			if (settings.isSnaplineOn)
				gpu.DrawLine({ wndSize.x >> 1, wndSize.y + 10 }, playerPos2D, color); // >> 1 == / 2

			// Draw ESP 2D Square
			if (settings.espMode == 1) {
				Vec2Int lefttop = headPos2D;
				Vec2Int rightBottom = playerPos2D;
				lefttop.x -= (rightBottom.y - lefttop.y) >> 2;
				rightBottom.x += (rightBottom.y - lefttop.y) >> 2;
				gpu.DrawSquare(lefttop, rightBottom, color); // Draw Square
			}
			else if (settings.espMode == 2) {
				// Draw ESP 3D
				Vec3 a, b, c, d, a1, b1, c1, d1; // 3D coords value
				Vec2Int sa, sb, sc, sd, sa1, sb1, sc1, sd1; // Screen Value
				a = playerPos;
				b.z = c.z = d.z = a.z;

				// The bottom rectangle calculation
				a.x += cosf(eyeAngle.y) * 5.f;
				a.y += sinf(eyeAngle.y) * 5.f;

				b.x = -a.x;
				b.y = a.y;

				c.x = -a.x;
				c.y = -a.y;

				d.x = a.x;
				d.y = -a.y;

				// The top Rectangle
				a1 = a;
				b1 = b;
				c1 = c;
				d1 = d;
				a1.z = b1.z = c1.z = d1.z = headPos.z;

				if (gpu.WorldToScreen(a, sa, VMatrix) && gpu.WorldToScreen(b, sb, VMatrix)
					&& gpu.WorldToScreen(c, sc, VMatrix) && gpu.WorldToScreen(d, sd, VMatrix)
					&& gpu.WorldToScreen(a1, sa1, VMatrix) && gpu.WorldToScreen(b1, sb1, VMatrix)
					&& gpu.WorldToScreen(c1, sc1, VMatrix) && gpu.WorldToScreen(d1, sd1, VMatrix))
				{
					gpu.DrawLine(sa, sb, color);
					gpu.DrawLine(sb, sc, color);
					gpu.DrawLine(sc, sd, color);
					gpu.DrawLine(sd, sa, color);

					gpu.DrawLine(sa, sa1, color);
					gpu.DrawLine(sb, sb1, color);
					gpu.DrawLine(sc, sc1, color);
					gpu.DrawLine(sd, sd1, color);

					gpu.DrawLine(sa1, sb1, color);
					gpu.DrawLine(sb1, sc1, color);
					gpu.DrawLine(sc1, sd1, color);
					gpu.DrawLine(sd1, sa1, color);
				}

				// HeadLine
				Vec3 endPos = gpu.TransformVec(oriHeadPos, eyeAngle, 60.f);
				Vec2Int oriPos2D, endPos2D;
				if (gpu.WorldToScreen(oriHeadPos, oriPos2D, VMatrix) && gpu.WorldToScreen(endPos, endPos2D, VMatrix))
					gpu.DrawLine(oriPos2D, endPos2D, color);

				// Velocity Line
				Vec3 entVelocity = ent.GetVelocity();
				Vec3 endPosVel = gpu.TransformVec(oriHeadPos, eyeAngle, 60.f);
				Vec2Int velPos2D, velPos3D;
				if (gpu.WorldToScreen(entVelocity, velPos2D, VMatrix) && gpu.WorldToScreen(endPosVel, velPos3D, VMatrix))
					gpu.DrawLine(velPos2D, velPos3D, color);
			}
		}
		catch (Entity::EntityException& e) {
			if (e.errCode == Entity::EntityException::EntityNotAvaliable)
				continue;
			else {
				e.Report();
				return;
			}
		}
	}
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
	// Update Weapon ID
	curWeapon.crosshairEntityId = (int*)((uintptr_t)base + netvars::m_iCrosshairId);
}
void ClientModule::Player::init() {
	// Update Offsets
	m_iHealth = (int*)(*base + netvars::m_iHealth);
	m_iTeamNum = (int*)(*base + netvars::m_iTeamNum);
	m_fFlags = (DWORD*)(*base + netvars::m_fFlags);
	m_iShotsFired = (int*)(*base + netvars::m_iShotsFired);
	m_vecVelocity = (Vec3*)(*base + netvars::m_vecVelocity);
	m_aimPunchAngle = (ViewAngle*)(*base + netvars::m_aimPunchAngle);
	m_dwBoneMatrix = (DWORD*)((uintptr_t)base + netvars::m_dwBoneMatrix);
	m_bDormant = (bool*)((uintptr_t)base + signatures::m_bDormant);
	m_vecOrigin = (Vec3*)((uintptr_t)base + netvars::m_vecOrigin);
	m_ArmorValue = (int*)((uintptr_t)base + netvars::m_ArmorValue);
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