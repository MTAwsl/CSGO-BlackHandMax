#include "stdafx.h"
#include "Hack.h"

// Hack functions
Cheat::Cheat() : gpu{ Graphics::Graphics(this) }, Aimbot(Aimbot::Aimbot(this)) {}

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
			if (!ent.isValidPlayer() || ent.GetBaseAddr() == modClient.localPlayer.GetBaseAddr()) // Entity 0 is localEnt (CWorld or localplayer I dont know)
				continue;
			Vec3 oriHeadPos;
			Vec3 headPos = oriHeadPos = ent.GetBonePos(8);
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

			if (settings.isESPTextOn) {
				std::stringstream s1, s2;
				s1 << ent.GetHealth();
				s2 << ent.GetArmorValue();
				std::string t1 = "hp: " + s1.str();
				std::string t2 = "ap: " + s2.str();
				char* healthMsg = (char*)t1.c_str();
				char* armorMsg = (char*)t2.c_str();

				gpu.DrawTextA(healthMsg, playerPos2D.x, playerPos2D.y, D3DCOLOR_ARGB(255, 255, 255, 255));
				gpu.DrawTextA(armorMsg, playerPos2D.x, playerPos2D.y + 12, D3DCOLOR_ARGB(255, 255, 255, 255));

				if (!ent.isHelmet())
					gpu.DrawTextA("Helmet", playerPos2D.x, playerPos2D.y + 24, D3DCOLOR_ARGB(255, 255, 255, 255));
			}
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
				float deltaAngle = eyeAngle.y + 45.f;
				if (deltaAngle > 360.f) deltaAngle -= 360.f;
				else if (deltaAngle < 0.f) deltaAngle += 360.f;
				float cosResult = cosf(TORAD(deltaAngle)) * 25.f;
				float sinResult = sinf(TORAD(deltaAngle)) * 25.f;
				a.x += cosResult;
				a.y += sinResult;

				b.x = playerPos.x - sinResult;
				b.y = playerPos.y + cosResult;

				c.x = playerPos.x - cosResult;
				c.y = playerPos.y - sinResult;

				d.x = playerPos.x + sinResult;
				d.y = playerPos.y - cosResult;

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
				Vec3 entVelocity = playerPos + ent.GetVelocity() * .25f;
				Vec3 endPosVel = gpu.TransformVec(oriHeadPos, eyeAngle, 60.f);
				Vec2Int velPos2D;
				if (gpu.WorldToScreen(entVelocity, velPos2D, VMatrix))
					gpu.DrawLine(velPos2D, playerPos2D, color);
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