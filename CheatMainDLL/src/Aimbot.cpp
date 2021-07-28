#include "stdafx.h"
#include "Aimbot.h"
#include "types.h"

Aimbot::Aimbot(Cheat* haxPtr) : pHax(haxPtr) {}

void Aimbot::run() {
	float bestFov = 120.0f;
	ViewAngleVec3 bestTargetAngle = {0, 0, 0};
	for (int i = 1; i < 32; i++) {
		try {
			Entity playerEnt = pHax->modClient.GetEntity(i);
			Vec3 headPos = playerEnt.GetBonePos(8);
			Vec3 eyePos = pHax->modClient.localPlayer.GetEyePos();

			if (!playerEnt.isValidPlayer() || playerEnt.GetBaseAddr() == pHax->modClient.localPlayer.GetBaseAddr() || playerEnt.GetBaseAddr() == pHax->modClient.GetEntity(0).GetBaseAddr()) continue;
			if (pHax->settings.Aimbot.friendlyFire && playerEnt.GetTeamNum() == pHax->modClient.localPlayer.GetTeamNum()) continue; // Friendly Fire

			ViewAngleVec3 targetAngle = { TODEG(atan2(-(headPos.z - eyePos.z), hypot(headPos.x - eyePos.x, headPos.y - eyePos.y))) , 
				TODEG(atan2(headPos.y - eyePos.y, headPos.x - eyePos.x)) , 0.0f};
			targetAngle = (targetAngle - pHax->modEngine.GetViewAngle() - pHax->modClient.localPlayer.GetAimPunchAngle() * 2).Normalize();
			float fov = hypot(targetAngle.x, targetAngle.y);
			if (fov < bestFov) {
				bestFov = fov;
				bestTargetAngle = targetAngle;
			}
		}
		catch (Entity::EntityException& e) {
			if (e.errCode != Entity::EntityException::EntityNotAvaliable)
				throw e;
			else
				continue;
		}
	}
	pHax->modEngine.SetViewAngle(pHax->modEngine.GetViewAngle() + bestTargetAngle / pHax->settings.Aimbot.smoothness);
}