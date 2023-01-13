#pragma once

struct Vec2 {
	float x, y;

	Vec2() {
		x = y = 0.0f;
	}

	Vec2(float x, float y) {
		this->x = x;
		this->y = y;
	}

	bool Compare(const Vec2& r) {
		return x == r.x && y == r.y;
	}
};

struct Vec3 {
	float x, y, z;

	Vec3() {
		x = y = z = 0.0f;
	}

	Vec3(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	bool Compare(const Vec3& r) {
		return x == r.x && y == r.y && z == r.z;
	}
};

struct Vec4 {
	float x, y, z, w;

	Vec4() {
		x = y = z = w = 0.0f;
	}

	Vec4(float x, float y, float z, float w) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}

	bool Compare(const Vec4& r) {
		return x == r.x && y == r.y && z == r.z && w == r.w;
	}
};
