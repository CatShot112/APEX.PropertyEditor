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
};

struct Mat3x3 {
	float a00, a01, a02;
	float b00, b01, b02;
	float c00, c01, c02;

	Mat3x3() {
		a00 = a01 = a02 = 0.0f;
		b00 = b01 = b02 = 0.0f;
		c00 = c01 = c02 = 0.0f;
	}

	Mat3x3(float a00, float a01, float a02, float b00, float b01, float b02, float c00, float c01, float c02) {
		this->a00 = a00;
		this->a01 = a01;
		this->a02 = a02;
		this->b00 = b00;
		this->b01 = b01;
		this->b02 = b02;
		this->c00 = c00;
		this->c01 = c01;
		this->c02 = c02;
	}
};
