#pragma once
struct Vec2Int {
	int x, y;
};

struct Vec2 {
	float x, y;
};

struct Vec3 {
	float x, y, z;
	Vec3 operator+(Vec3 d) {
		return { x + d.x, y + d.y, z + d.z };
	}
	Vec3 operator-(Vec3 d) {
		return { x - d.x, y - d.y, z - d.z };
	}
	Vec3 operator*(float d) {
		return { x * d, y * d, z * d };
	}
};

typedef struct ViewAngleVec3 {
	float x, y, z;

	ViewAngleVec3 operator+(ViewAngleVec3 d) {
		return { x + d.x, y + d.y, z + d.z };
	}
	ViewAngleVec3 operator-(ViewAngleVec3 d) {
		return { x - d.x, y - d.y, z - d.z };
	}
	ViewAngleVec3 operator*(float d) {
		return { x * d, y * d, z * d };
	}

	ViewAngleVec3 Normalize() {
		// Fix any invalid angles to prevent untrusted ban
		while (y < -180) { y += 360; }
		while (y > 180) { y -= 360; }
		if (x > 89) { x = 89; }
		if (x < -89) { x = -89; }
		return *this;
	}
} ViewAngle;